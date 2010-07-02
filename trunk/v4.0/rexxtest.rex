/* sample1.rex - Script básico para conectar a um host e obter um campo da tela */

/* Carrega a API 3270 */
call rxfuncadd 'rx3270LoadFuncs', 'rx3270', 'rx3270LoadFuncs'
retc = rx3270LoadFuncs()
say 'Return code from loading rx3270 functions was' retc

/* Se estiver conectado desconecta */
if rx3270QueryCState() = "CONNECTED_TN3270E"
	then ok = rx3270Disconnect()

/* Reconecta ao novo host, espera pela negociacao */
ok = rx3270Connect("MODO:HOSTNAME:PORTA",1)

if rx3270QueryCState() <> "CONNECTED_TN3270E" then
do
	say "Não foi possível conectar ao host, cancelando"
	return 0
end

/* Obtem 40 caracteres a partir da linha 14, coluna 22 */
SCREEN_CONTENTS = rx3270ReadScreen(14,22,40)

say "O conteudo da tela e: " SCREEN_CONTENTS

