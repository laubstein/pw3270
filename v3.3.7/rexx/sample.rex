
call rx3270Popup "Sample Rexx script running on g3270 "||rx3270Version()||" (Status: "||rx3270QueryCState()||")"
ok = rx3270SetVisible(1)
ok = rx3270SendENTERKey()

ok = rx3270WaitForString("c1103788")

say "Wait saiu com "||ok

return 0
