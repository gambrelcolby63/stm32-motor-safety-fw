*** Variables ***
${ELF}          ${CURDIR}/../../build/fw/gimbal.elf
${PLATFORM}     ${CURDIR}/stm32f407_gimbal.repl

*** Keywords ***
Create Gimbal
    Execute Command          mach create
    Execute Command          machine LoadPlatformDescription @${PLATFORM}
    Execute Command          sysbus LoadELF @${ELF}
    # Patterns use '.' for '=' because the UART keyword treats '=' as a named argument.
    Create Terminal Tester   sysbus.usart2  defaultPauseEmulation=true  timeout=8

Expect
    [Arguments]              ${pattern}    ${timeout}=8
    Wait For Line On Uart    ${pattern}    timeout=${timeout}    treatAsRegex=true

# The terminal tester's line writer waits on an echo this firmware does not send.
# Push the bytes into the USART receive queue and let the baud clock deliver them.
Send Line
    [Arguments]              ${text}
    ${bytes}=                Evaluate    [ord(c) for c in $text] + [10]
    FOR    ${byte}    IN    @{bytes}
        Execute Command      sysbus.usart2 WriteChar ${byte}
    END
    Execute Command          emulation RunFor "0.02"

Home Both Axes
    Send Line                home
    Expect                   OK home
    Expect                   phase.tilt-seek
    Execute Command          sysbus.gpioPortB.tiltHomeButton Press
    Expect                   phase.tilt-leave
    Execute Command          sysbus.gpioPortB.tiltHomeButton Release
    Expect                   phase.pan-seek
    Execute Command          sysbus.gpioPortB.panHomeButton Press
    Expect                   phase.pan-leave
    Execute Command          sysbus.gpioPortB.panHomeButton Release
    Expect                   homed.1

*** Test Cases ***
Boot Stays Safe Until Homed
    [Tags]                   smoke
    Create Gimbal
    Start Emulation
    Expect                   BOOT pan-tilt-gimbal
    Expect                   state.SAFE
    Send Line                enable
    Expect                   ERR not-homed

Homing Then Enable
    Create Gimbal
    Start Emulation
    Expect                   BOOT pan-tilt-gimbal
    Home Both Axes
    Send Line                enable
    Expect                   OK enable
    Expect                   state.ENABLED

Estop Latches Until Released And Reset
    Create Gimbal
    Start Emulation
    Expect                   BOOT pan-tilt-gimbal
    Home Both Axes
    Send Line                enable
    Expect                   state.ENABLED
    Execute Command          sysbus.gpioPortC.estopButton Press
    Expect                   state.ESTOP
    Send Line                reset
    Expect                   OK reset
    Run Keyword And Expect Error
    ...                      *Terminal tester failed*
    ...                      Expect    state.SAFE    timeout=0.3
    # A failed wait leaves emulation running, and RunFor refuses that.
    Execute Command          pause
    Execute Command          sysbus.gpioPortC.estopButton Release
    Send Line                reset
    Expect                   OK reset
    Expect                   state.SAFE.*homed.0

Driver Fault Is Overcurrent
    Create Gimbal
    Start Emulation
    Expect                   BOOT pan-tilt-gimbal
    Expect                   state.SAFE
    Execute Command          sysbus.gpioPortD.panDriverFault Press
    Expect                   fault.OVERCURRENT

Limit Switch While Enabled
    Create Gimbal
    Start Emulation
    Expect                   BOOT pan-tilt-gimbal
    Home Both Axes
    Send Line                enable
    Expect                   state.ENABLED
    Execute Command          sysbus.gpioPortB.tiltLimitMaxButton Press
    Expect                   fault.LIMIT

Stall When The Encoder Does Not Move
    Create Gimbal
    Start Emulation
    Expect                   BOOT pan-tilt-gimbal
    Home Both Axes
    Send Line                enable
    Expect                   state.ENABLED
    Send Line                rate 30 0
    Expect                   OK rate
    Expect                   state.MOVING
    Execute Command          emulation RunFor "0.4"
    Send Line                hb
    Expect                   OK hb
    Execute Command          emulation RunFor "0.4"
    Send Line                hb
    Expect                   OK hb
    Execute Command          emulation RunFor "0.4"
    Send Line                hb
    Expect                   OK hb
    Execute Command          emulation RunFor "0.4"
    Expect                   fault.STALL

Heartbeat Loss Is A Controlled Stop
    Create Gimbal
    Start Emulation
    Expect                   BOOT pan-tilt-gimbal
    Home Both Axes
    Send Line                enable
    Expect                   state.ENABLED
    Execute Command          emulation RunFor "0.6"
    Expect                   state.SAFE.*fault.NONE

Watchdog Kick Keeps The Controller Alive
    [Tags]                   watchdog
    Create Gimbal
    Start Emulation
    Expect                   BOOT pan-tilt-gimbal
    Expect                   state.SAFE
    Execute Command          emulation RunFor "35"
    Run Keyword And Expect Error
    ...                      *Terminal tester failed*
    ...                      Expect    BOOT pan-tilt-gimbal    timeout=0
    Send Line                status
    Expect                   OK state.SAFE
