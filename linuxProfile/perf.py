from linuxProfile.api import sampling as s

HW_CPU_CYCLES = (s.TYPE_HARDWARE, s.COUNT_HW_CPU_CYCLES)
SW_CPU_CLOCK = (s.TYPE_SOFTWARE, s.COUNT_SW_CPU_CLOCK)

if __name__ == '__main__':
    from linuxProfile import launch
    launch.run(launch.ProfileRunnerPerf())
