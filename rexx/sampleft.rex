/*
 * Sample ultra-simples para envio de varios arquivos...
 *
 */

 /* Verifica se esta conectado ao host */
 if rx3270QueryCState() <> "CONNECTED_TN3270E" then
 do
	ok = rx3270Popup("Nao ha conexao ativa")
	return 0
 end

 /* Primeiro cria um vetor com os arquivos a enviar */

 lista = .array~new(3)

 /* A primeira palavra contem o arquivo origem, a segunda palavra o arquivo destino */
 lista[1] = "Arquivo_local_1 arq1"
 lista[2] = "Arquivo_local_2 arq2"
 lista[3] = "Arquivo_local_3 arq3"
 
 do f = 1 to lista~items
	status = rx3270BeginFileSend(word(lista[f],1),word(lista[f],2))

	if status = 0 then
	do
		/* Transferencia do arquivo iniciou, espera terminar */
		ok = rx3270WaitForFTComplete()
	end	
	else
	do
		say "Envio de "||word(lista[f],1)||" para "||word(lista[f],2)||" saiu com "||status
	end

 end

 ok = rx3270Popup("Script terminado")

return 0

