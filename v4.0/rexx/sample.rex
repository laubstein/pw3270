
trace "?R"
call rx3270Popup "Sample Rexx script running on g3270 "||rx3270Version()||" (Status: "||rx3270QueryCState()||")"
ok = rx3270SetVisible(1)
ok = rx3270SendENTERKey()

return 0
