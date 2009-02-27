
say rx3270ReadScreen()

call rx3270Popup "Sample Rexx script running on g3270 "||rx3270Version()||" (Status: "||rx3270QueryCState()||")"

ok = rx3270MoveCursor(14,21)
ok = rx3270SetVisible(1)
ok = rx3270SendENTERKey()
ok = rx3270Sleep(5)
ok = rx3270Quit()

return 0
