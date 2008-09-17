
say rx3270ReadScreen()

call rx3270Popup "Sample Rexx script running on g3270 "||rx3270Version()||" (Status: "||rx3270QueryCState()||")"


return 0
