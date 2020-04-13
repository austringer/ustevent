import gdb
import re

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
            backtrace       = debug_info['_backtrace']
            size            = int(debug_info['_size'])

            print('Fiber {0} {1} {2}'.format(count + 1, hex(context_pointer), description))

            pretty_stack(backtrace, size)
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
            backtrace       = debug_info['_backtrace']
            size            = int(debug_info['_size'])

            print("stack size", size)

            print('Fiber {0} {1} {2}'.format(count + 1, hex(context_pointer), description))

            pretty_stack(backtrace, size)
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
