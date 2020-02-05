import gdb
import re
import ctypes

libunwind = ctypes.CDLL('libunwind.so')

class stack_t(ctypes.Structure):
    pass
stack_t._fields_ = [
    ('ss_sp',       ctypes.c_void_p),
    ('ss_flags',    ctypes.c_int),
    ('ss_size',     ctypes.c_size_t)
]

class _libc_fpxreg(ctypes.Structure):
    pass
_libc_fpxreg._fields_ = [
    ('significand', ctypes.ARRAY(ctypes.c_ushort, 4)),
    ('exponent',    ctypes.c_ushort),
    ('__glibc_reserved1', ctypes.ARRAY(ctypes.c_ushort, 3))
]

class _libc_xmmreg(ctypes.Structure):
    pass
_libc_xmmreg._fields_ = [
    ('element',     ctypes.ARRAY(ctypes.c_uint32, 4))
]

class _libc_fpstate(ctypes.Structure):
    pass
_libc_fpstate._fields_ = [
    ('cwd',         ctypes.c_uint16),
    ('swd',         ctypes.c_uint16),
    ('ftw',         ctypes.c_uint16),
    ('fop',         ctypes.c_uint16),
    ('rip',         ctypes.c_uint64),
    ('rdp',         ctypes.c_uint64),
    ('mxcsr',       ctypes.c_uint32),
    ('mxcr_mask',   ctypes.c_uint32),
    ('_st',         ctypes.ARRAY(_libc_fpxreg, 8)),
    ('_xmm',        ctypes.ARRAY(_libc_xmmreg, 16)),
    ('__glibc_reserved1', ctypes.ARRAY(ctypes.c_uint32, 24))
]

class mcontext_t(ctypes.Structure):
    pass
mcontext_t._fields_ = [
    ('gregs',   ctypes.ARRAY(ctypes.c_longlong, 23)),
    ('fpregs',  ctypes.POINTER(_libc_fpstate)),
    ('__reserved1', ctypes.ARRAY(ctypes.c_ulonglong, 8))
]

class sigset_t(ctypes.Structure):
    pass
sigset_t._fields_ = [
    ('__val', ctypes.ARRAY(ctypes.c_ulong, 16))
]

class ucontext_t(ctypes.Structure):
    pass
ucontext_t._fields_ = [
    ('uc_flags',        ctypes.c_ulong),
    ('uc_link',         ctypes.POINTER(ucontext_t)),
    ('uc_stack',        stack_t),
    ('uc_mcontext',     mcontext_t),
    ('uc_sigmask',      sigset_t),
    ('__fpregs_mem',    _libc_fpstate)
]

class unw_cursor_t(ctypes.Structure):
    pass
unw_cursor_t._fields_ = [
    ('opaque', ctypes.ARRAY(ctypes.c_uint64, 127))
]

unw_init_local = libunwind._ULx86_64_init_local

unw_step = libunwind._ULx86_64_step
unw_step.restype = ctypes.c_int

unw_get_reg = libunwind._ULx86_64_get_reg

line_pattern    = re.compile(r'Line (\d+) of \"(.*)\" starts at address 0x[0-9a-f]+ <(.*)\+\d+> and ends at .*')
symbol_pattern  = re.compile(r'(.*) \+ \d+ in .* of (.*)')
bt_pattern      = re.compile(r'#\d+ +(0x[0-9a-f]+) in .*')

def get_data_from_atomic(atomic):
    return atomic['_M_i']

def get_data_from_atomic_pointer(atomic):
    return atomic['_M_b']['_M_p']

def get_scheduler():
    return gdb.parse_and_eval('::boost::fibers::context::active().scheduler_')

def get_strategy(scheduler):
    strategy_type = gdb.lookup_type('::ustevent::ContextStrategy')
    return scheduler['algo_']['px'].cast(strategy_type.pointer()).dereference()

def is_ustevent_thread(strategy):
    magic = strategy['_magic']
    return int(magic) == 0x544E455645545355

def get_backtrace_in_running_fiber():
    bt = gdb.execute('bt', to_string=True).splitlines()
    backtrace = []
    for line in bt:
        bt_match = bt_pattern.match(line)
        if bt_match:
            address = bt_match.group(1)
            backtrace.append(int(address, 16))
    return backtrace

def get_debug_infos_in_waiting_fibers(strategy):
    header = strategy['_waiting_fibers_head']
    next_info = header
    debug_infos = []
    while True:
        next_info = next_info['_next']
        if int(next_info) == 0:
            break
        debug_infos.append(next_info)
    return debug_infos

def get_debug_infos_in_ready_fibers(strategy):
    ready_queue = strategy['_ready_queue']
    top = get_data_from_atomic(ready_queue['top_'])
    bottom = get_data_from_atomic(ready_queue['bottom_'])
    array = get_data_from_atomic_pointer(ready_queue['array_'])
    capacity = array['capacity_']
    storage = array['storage_']
    atomic_context_pointer_type = gdb.lookup_type('::std::atomic<::boost::fibers::context*>').pointer()
    debug_info_pointer_type = gdb.lookup_type('::ustevent::FiberDebugInfo').pointer()
    debug_infos = []
    # loop for [top, bottom)
    for i in range(top, bottom):
        slot = storage[i % int(capacity)]
        atomic_context = slot.address.reinterpret_cast(atomic_context_pointer_type)
        atomic_context = atomic_context.dereference()
        context = get_data_from_atomic_pointer(atomic_context).dereference()
        context_type = context['type_']
        if context_type == 8:   # boost::fibers::type::worker_context
            properties = context['properties_']
            if int(properties) != 0:
                debug_info = properties.cast(debug_info_pointer_type).dereference()
                debug_infos.append(debug_info)
    return debug_infos

def pretty_frame(frame, i):
    # go back 0x4 bytes to back to the previous line
    line_info = gdb.execute('info line *{0}'.format(int(frame) - 0x4), to_string=True)
    line_match = line_pattern.match(line_info)
    if line_match:
        line = line_match.group(1)
        file = line_match.group(2)
        function = line_match.group(3)
        return '#{0} 0x{1} in {2} at {3}:{4}'.format(
            str(i - 0).zfill(2), hex(frame)[2:].zfill(16),
            function, file, line
        )

    symbol_info = gdb.execute('info symbol {0}'.format(int(frame)), to_string=True)
    symbol_match = symbol_pattern.match(symbol_info)
    if symbol_match:
        function = symbol_match.group(1)
        elf = symbol_match.group(2)
        return '#{0} 0x{1} in {2} from {3}'.format(
            str(i - 0).zfill(2), hex(frame)[2:].zfill(16),
            function, elf
        )
    return '#{0} 0x{1} in ?? ()'.format(
        str(i - 0).zfill(2), hex(frame)[2:].zfill(16),
    )

def pretty_stack(backtrace, size):
    i = 0
    frame_number = 0
    start_print = False
    for i in range(size):
        frame = pretty_frame(backtrace[i], frame_number)
        if start_print:
            print(frame)
            frame_number += 1
        elif 'ustevent::FiberDebugInfo' not in frame and 'ustevent::ContextStrategy' not in frame:
            # skip ustevent::FiberDebugInfo::backtrace()
            #      ustevent::ContextStrategy::_recordBacktrace()
            #      ustevent::ContextStrategy::pick_next()
            start_print = True
            print(frame)
            frame_number += 1

def backtrace_unwind_context(unwind_context):
    cursor = unw_cursor_t()
    unw_init_local(ctypes.addressof(cursor), unwind_context.address)
    rip = ctypes.c_uint64()

    n = 0
    while unw_step(ctypes.addressof(cursor)).value != 0:
        unw_get_reg(ctypes.addressof(cursor), ctypes.c_int(16), ctypes.addressof(rip)) # UNW_REG_IP = 16
        pretty_frame(rip.value, n)
        n += 1

def print_active_fiber(count):
    print('Active Fiber:')
    active_context_pointer = gdb.parse_and_eval('::boost::fibers::context::active()')
    description = ''
    properties = active_context_pointer.dereference()['properties_']
    if int(properties) != 0:
        debug_info_pointer_type = gdb.lookup_type('::ustevent::FiberDebugInfo').pointer()
        debug_info = properties.cast(debug_info_pointer_type).dereference()
        description = debug_info['_description']

    print('Fiber {0} {1} {2}'.format(count + 1, hex(active_context_pointer), description))

    bt = get_backtrace_in_running_fiber()
    pretty_stack(bt, len(bt))
    count += 1
    print('')
    return count

def print_waiting_fibers(count):
    scheduler = get_scheduler()
    strategy = get_strategy(scheduler)
    if is_ustevent_thread(strategy):
        print('Waiting Fibers:')
        waiting_fiber_debug_infos = get_debug_infos_in_waiting_fibers(strategy)
        print(len(waiting_fiber_debug_infos))
        for debug_info in waiting_fiber_debug_infos:
            context_pointer = debug_info['ctx_']
            description     = debug_info['_description']
            # backtrace       = debug_info['_backtrace']
            # size            = int(debug_info['_size'])
            unwind_context  = debug_info['_unwind_context']

            print('Fiber {0} {1} {2}'.format(count + 1, hex(context_pointer), description))

            # pretty_stack(backtrace, size)
            backtrace_unwind_context(unwind_context)
            count += 1
            print('')
    return count

def print_ready_fibers(count):
    scheduler = get_scheduler()
    strategy = get_strategy(scheduler)
    if is_ustevent_thread(strategy):
        print('Ready Fibers:')
        ready_fiber_debug_infos = get_debug_infos_in_ready_fibers(strategy)
        print(len(ready_fiber_debug_infos))
        for debug_info in ready_fiber_debug_infos:
            context_pointer = debug_info['ctx_']
            description     = debug_info['_description']
            # backtrace       = debug_info['_backtrace']
            # size            = int(debug_info['_size'])
            unwind_context  = debug_info['_unwind_context']

            print('Fiber {0} {1} {2}'.format(count + 1, hex(context_pointer), description))

            # pretty_stack(backtrace, size)
            backtrace_unwind_context(unwind_context)
            count += 1
            print('')
    return count

class backtrace_fiber(gdb.Command):
    def __init__(self):
        super(backtrace_fiber, self).__init__("bf", gdb.COMMAND_DATA)

    def invoke(self, arg, from_tty):
        count = 0

        count = print_active_fiber(count)

        count = print_waiting_fibers(count)

        print_ready_fibers(count)


backtrace_fiber()
