"""
GDB commands for various HHVM ID lookups.
"""
# @lint-avoid-python-3-compatibility-imports

import gdb
import stl
from gdbutils import *
from unit import curunit


#------------------------------------------------------------------------------
# `lookup' command.

class LookupCommand(gdb.Command):
    """Lookup HHVM runtime objects by ID."""

    def __init__(self):
        super(LookupCommand, self).__init__('lookup', gdb.COMMAND_DATA,
                                            gdb.COMPLETE_NONE, True)

LookupCommand()


#------------------------------------------------------------------------------
# `lookup func' command.

class LookupFuncCommand(gdb.Command):
    """Lookup a Func* by its FuncId."""

    def __init__(self):
        super(LookupFuncCommand, self).__init__('lookup func',
                                                gdb.COMMAND_DATA)

    def invoke(self, args, from_tty):
        argv = parse_argv(args)

        if len(argv) != 1:
            print 'Usage: lookup func <FuncId>'
            return

        funcid = argv[0].cast(T('HPHP::FuncId'))
        func_vec = V('HPHP::s_funcVec')['m_vals']['_M_t']['_M_head_impl']

        func = func_vec[funcid]['_M_b']['_M_p']
        gdb.execute('print (%s)%s' % (str(func.type), str(func)))

LookupFuncCommand()


#------------------------------------------------------------------------------
# `lookup litstr' command.

def lookup_litstr(litstr_id, unit):
    table = None
    gloff = V('HPHP::kGlobalLitstrOffset')

    if litstr_id >= gloff:
        litstr_id -= gloff
        unit = V('HPHP::LitstrTable::s_litstrTable')

    return stl.vector_at(unit['m_namedInfo'], litstr_id)['first']


class LookupLitstrCommand(gdb.Command):
    """Lookup a litstr StringData* by its Unit* and Id."""

    def __init__(self):
        super(LookupLitstrCommand, self).__init__('lookup litstr',
                                                  gdb.COMMAND_DATA)

    def invoke(self, args, from_tty):
        argv = parse_argv(args)

        if len(argv) == 0 or len(argv) > 2:
            print 'Usage: lookup litstr <Id> [Unit*]'
            return

        if len(argv) == 1:
            if curunit is None:
                print 'lookup litstr: No Unit set or provided.'
            unit = curunit

        unit = argv[0].cast(T('HPHP::Unit').pointer())
        litstr_id = argv[1].cast(T('HPHP::Id'))

        litstr = lookup_litstr(litstr_id, unit)
        gdb.execute('print (%s)%s' % (str(litstr.type), str(litstr)))

LookupLitstrCommand()
