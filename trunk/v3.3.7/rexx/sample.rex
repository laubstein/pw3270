
say "Screen contents at 82 are "||rx3270ReadScreen(81,20)

call rx3270Popup "Sample Rexx script running on g3270 "||rx3270Version()||" (Status: "||rx3270QueryCState()||")"


return 0
