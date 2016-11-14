

.include "DP256Reg.s"

_MotorSTART::

	movb #$FF, PWMPOL
	
	bset PWMCLK, #$10
	movb #$70, PWMPRCLK
	
	movb #0, PWMSCLA
	bclr PWMCAE, #$7F
	bclr PWMCTL, #$F3

	movb #100, PWMPER7
	movb #$80, PWME
	
	bset DDRP, #$60
	movb #$20, PTP

	movb #$20, PWMDTY7

	rts
	
_MotorSTOP::

	movb #$0, PWMDTY7
	
	rts