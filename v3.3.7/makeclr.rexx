
/*
!   Each scheme is a list of 23 items:
!    1  X color to use for IBM "neutral/black" (also used as ANSI color 0)
!    2	X color to use for IBM "blue" (also used for ANSI color 4)
!    3	X color to use for IBM "red" (also used for ANSI color 1)
!    4	X color to use for IBM "pink" (also used for ANSI color 5)
!    5	X color to use for IBM "green" (also used for ANSI color 2)
!    6	X color to use for IBM "turquoise"
!    7	X color to use for IBM "yellow" (also used for ANSI color 3)
!    8	X color to use for IBM "neutral/white"
!    9	X color to use for IBM "black"
!    10 X color to use for IBM "deep blue"
!    11	X color to use for IBM "orange"
!    12	X color to use for IBM "purple"
!    13	X color to use for IBM "pale green"
!    14	X color to use for IBM "pale turquoise" (also used for ANSI color 6)
!    15	X color to use for IBM "grey"
!    16	X color to use for IBM "white" (also used for ANSI color 7)
!    17 X color to use if one of 0..15 cannot be allocated (white or black)
!    18 X color to use as the default screen background
!    19 X color to use as the select background
!    20 IBM color index (0..15) to use for unprotected, unhighlighted fields
!    21 IBM color index (0..15) to use for unprotected, highlighted fields
!    22 IBM color index (0..15) to use for protected, unhighlighted fields
!    23 IBM color index (0..15) to use for protected, highlighted fields
!
*/

ok = ConvertColor("X3270 black deepSkyBlue red pink green turquoise yellow white black blue3 orange purple paleGreen paleTurquoise2 grey white white black dimGrey 4 2 1 15")

/*
ok = ConvertColor("Reverse black blue firebrick pink green4 cadetBlue goldenrod black black blue3 orange purple paleGreen darkTurquoise grey black black white dimGrey 4 2 1 0")
ok = ConvertColor("Bright black blue red magenta green turquoise yellow white black blue3 orange purple paleGreen cyan grey white white black dimGrey 4 2 1 15")
ok = ConvertColor("CPE black LightBlue1 PaleVioletRed1 pink green turquoise yellow white black LightBlue3 orange MediumPurple1 paleGreen paleTurquoise2 grey80 white white black dimGrey 4 2 1 15")
ok = ConvertColor("Green green green green green green green green green green green green green green green green green white black dimGrey 4 15 4 15")
*/

return 0

ConvertColor: procedure
	parse arg list

	name = "TERMINAL_COLOR_BACKGROUND TERMINAL_COLOR_BLUE TERMINAL_COLOR_RED TERMINAL_COLOR_PINK TERMINAL_COLOR_GREEN TERMINAL_COLOR_TURQUOISE TERMINAL_COLOR_YELLOW TERMINAL_COLOR_WHITE TERMINAL_COLOR_BLACK TERMINAL_COLOR_DARK_BLUE TERMINAL_COLOR_ORANGE TERMINAL_COLOR_PURPLE TERMINAL_COLOR_DARK_GREEN TERMINAL_COLOR_DARK_TURQUOISE TERMINAL_COLOR_MUSTARD TERMINAL_COLOR_GRAY TERMINAL_COLOR_FIELD_DEFAULT TERMINAL_COLOR_FIELD_INTENSIFIED TERMINAL_COLOR_FIELD_PROTECTED TERMINAL_COLOR_FIELD_PROTECTED_INTENSIFIED TERMINAL_COLOR_SELECTED_BG TERMINAL_COLOR_SELECTED_FG TERMINAL_COLOR_CURSOR TERMINAL_COLOR_CROSS_HAIR TERMINAL_COLOR_OIA_BACKGROUND TERMINAL_COLOR_OIA TERMINAL_COLOR_OIA_SEPARATOR TERMINAL_COLOR_OIA_STATUS_OK TERMINAL_COLOR_OIA_STATUS_INVALID"

	clr.0 = words(list)-1

	do f = 1 to clr.0
		clr.f = word(list,f+1)
	end

	say '{	N_( "'||word(list,1)||'" ),					"'||clr.1||',"		// '||word(name,1)

	do f = 2 to 16
		say '									"'||clr.f||',"		// '||word(name,f)
	end

	say ''

	do f = 1 to 4
		x = 19+f
		p = clr.x
		say '									"'||clr.p||',"		// '||word(name,16+f)
	end

	say ''

	ls = "19 1 8 8 1"
	do f = 1 to words(ls)
		p = word(ls,f)
		say '									"'||clr.p||',"		// '||word(name,20+f)
	end

	p = 17
	do f = 1 to 3
		say '									"'||clr.p||',"		// '||word(name,25+f)
	end
	say '									"'||clr.p||'"		// '||word(name,25+f)
	say '}'
	say ''
	say ''
return 0
