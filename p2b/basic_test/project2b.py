import toolspath
from testing import Xv6Build, Xv6Test

'''
Basic System Call Tests
'''

class Scheduling1(Xv6Test):
   name = "getpinfo"
   description = "Call getpinfo() from a user program \n The corresponding test file is at ~cs537-1/ta/tests/2b-new/ctests/" + name + ".c"
   tester = "ctests/" + name + ".c"
   make_qemu_args = "CPUS=1"
   point_value = 10
   timeout = 30

class Scheduling2(Xv6Test):
   name = "setticket"
   description = "Call settickets() from a user program \n The corresponding test file is at ~cs537-1/ta/tests/2b-new/ctests/" + name + ".c"
   tester = "ctests/" + name + ".c"
   make_qemu_args = "CPUS=1"
   point_value = 10
   timeout = 30

'''
Basic Scheduling Functionality Tests
'''
class Scheduling3(Xv6Test):
   name = "processesinuse"
   description = "Check the number of processes in use by calling getpinfo() \n The corresponding test file is at ~cs537-1/ta/tests/2b-new/ctests/" + name + ".c"
   tester = "ctests/" + name + ".c"
   make_qemu_args = "CPUS=1"
   point_value = 20
   timeout = 30

class Scheduling4(Xv6Test):
   name = "default_tickets"
   description = "Check the default tickets set for processes \n The corresponding test file is at ~cs537-1/ta/tests/2b-new/ctests/" + name + ".c"
   tester = "ctests/" + name + ".c"
   make_qemu_args = "CPUS=1"
   point_value = 10
   timeout = 30

class Scheduling5(Xv6Test):
   name = "high_sleep"
   description = "Check that a process with a low number of tickets runs when a process with a high number of tickets sleeps \n The corresponding test file is at ~cs537-1/ta/tests/2b-new/ctests/" + name + ".c"
   tester = "ctests/" + name + ".c"
   make_qemu_args = "CPUS=1"
   point_value = 20
   timeout = 60

class Scheduling6(Xv6Test):
   name = "forktickets"
   description = "Check that a child process has the same number of tickets as its parent \n The corresponding test file is at ~cs537-1/ta/tests/2b-new/ctests/" + name + ".c"
   tester = "ctests/" + name + ".c"
   make_qemu_args = "CPUS=1"
   point_value = 20
   timeout = 30

'''
Lottery Scheduling Tests
'''
class Scheduling7(Xv6Test):
   name = "high_tickets"
   description = "Check that a process with an extremely high number of tickets is more likely to be scheduled \n The corresponding test file is at ~cs537-1/ta/tests/2b-new/ctests/" + name + ".c"
   tester = "ctests/" + name + ".c"
   make_qemu_args = "CPUS=1"
   point_value = 10
   timeout = 60

class Scheduling8(Xv6Test):
   name = "switch_tickets"
   description = "A high ticket process will switch with a low ticket process midway through \n The corresponding test file is at ~cs537-1/ta/tests/2b-new/ctests/" + name + ".c"
   tester = "ctests/" + name + ".c"
   make_qemu_args = "CPUS=1"
   point_value = 20
   timeout = 60

'''
Fancy Tests
'''

class Scheduling9(Xv6Test):
   name = "feedback_converge"
   description = "Processes will attempt to adjust their tickets to achieve equal runtime \n The corresponding test file is at ~cs537-1/ta/tests/2b-new/ctests/" + name + ".c"
   tester = "ctests/" + name + ".c"
   make_qemu_args = "CPUS=1"
   point_value = 20
   timeout = 120

class Scheduling10(Xv6Test):
   name = "feedback_diverge"
   description = "Processes will attempt to adjust their tickets to achieve unequal runtime \n The corresponding test file is at ~cs537-1/ta/tests/2b-new/ctests/" + name + ".c"
   tester = "ctests/" + name + ".c"
   make_qemu_args = "CPUS=1"
   point_value = 20
   timeout = 120

class Scheduling11(Xv6Test):
   name = "stress_equal"
   description = "Check that several processes with the same number of tickets are equally scheduled \n The corresponding test file is at ~cs537-1/ta/tests/2b-new/ctests/" + name + ".c"
   tester = "ctests/" + name + ".c"
   make_qemu_args = "CPUS=1"
   point_value = 20
   timeout = 120

class Scheduling12(Xv6Test):
   name = "stress_manylow"
   description = "A process with a high number of tickets should run as often as the total of many processes whose tickets sum to that number \n The corresponding test file is at ~cs537-1/ta/tests/2b-new/ctests/" + name + ".c"
   tester = "ctests/" + name + ".c"
   make_qemu_args = "CPUS=1"
   point_value = 20
   timeout = 120

class Scheduling13(Xv6Test):
   name = "random"
   description = "Check that the scheduling is random, not just uniform \n The corresponding test file is at ~cs537-1/ta/tests/2b-new/ctests/" + name + ".c"
   tester = "ctests/" + name + ".c"
   make_qemu_args = "CPUS=1"
   point_value = 30
   timeout = 30

import toolspath
from testing.runtests import main
main(Xv6Build, [Scheduling1, Scheduling2, Scheduling3, Scheduling4, Scheduling5, Scheduling6, Scheduling7, Scheduling8, Scheduling9, Scheduling10, Scheduling11,Scheduling12,Scheduling13])
