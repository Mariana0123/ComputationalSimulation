// CONTROLADOR DE MALHA FECHADA
# include <stdio.h>
# include <locale.h>
# include <stdlib.h>
# include <math.h>

int main(){
	
	setlocale(LC_ALL, "Portuguese");
	
//PARTE 1 - DECLARAÇÃO DAS VARIÁVEIS
	
	//Equação da reta afim do fluxo ideal:
	float freqcardrepouso, freqcardexercicio, fluxocoracaoemrepouso, fluxocoracaoexercicio, coeficientedareta; 
	//Valores desejados de controle:
	float Potmaxdesejada, fluxodesejado, P2Pdesejado;
	//Estados iniciais do paciente simulado:
	float estadocardiacodopaciente, estadodefluxodabomba, estadoderesistencia = 0;
	//Variáveis utilizadas para simular o paciente:	
	float freqseg, tempodeamostragem[1000], pressaosimulada[1000], fluxosimulado[1000],pressaosimuladaIC[1000], fluxosimuladoIC[1000];
	//Variáveis de realimentação do controle:
	float pressaoreal[1000], fluxoreal[1000], Potreal, P2Preal, rotacaoreal, correntereal; 
	//Variáveis de estimadores do paciente:
	int picomaximofcard = -1000, contadordepicofcard = 0;
	float picomaximoflu = 0, picominimoflu = 0, constantederotacao = 0;
	//Ganhos do controle de velocidade:
	float kp1 = 400, kp2 = 83.3, kp3 = 600, kp4 = 800;
	//Saídas do controle:
	float Xpot, XP2P, Xfluxo[1000], Xfluxominimo[1000], Xvelocidade, Xseccao1, Xfluxovelocidadedabomba = 0;

//PARTE 2 - SOLICITAR AS VARIÁVEIS DE ENTRADA

	printf("Médico, entre com a frequência cardíaca durante o repouso: \n(exemplo: 30)\n");
	scanf("%f", &freqcardrepouso);
	printf("\nEntre com a frequência cardíaca durante o exercício: \n(exemplo: 120)\n");
	scanf("%f", &freqcardexercicio);
	printf("\nEntre com o fluxo em exercício: \n(exemplo: 10)\n");
	scanf("%f", &fluxocoracaoexercicio);
	printf("\nEntre com o valor de fluxo em repouso: \n(exemplo: 3)\n");
	scanf("%f", &fluxocoracaoemrepouso);
	printf("\nEntre com o estado de fluxo da bomba: \n(exemplo: 4)\n");
	scanf("%f", &estadodefluxodabomba);
	rotacaoreal = (4000 * estadodefluxodabomba)/10;
	printf("\nEntre com o estado cardíaco do paciente: \n(exemplo: 60)\n");
	scanf("%f", &estadocardiacodopaciente);
	freqseg = estadocardiacodopaciente/60;
	//a potência máxima da bomba corresponde ao maior valor que a bomba pode consumir de energia.
	printf("\nEntre com a potência máxima desejada da bomba: \n(exemplo: 48)\n");
	scanf("%f", &Potmaxdesejada);
	printf("\nEntre com o fluxo P2P desejado da bomba: \n(exemplo: 2)\n");
	scanf("%f", &P2Pdesejado);
	printf("\n");
	
//PARTE 3 - SIMULAR UM CORAÇÃO SAUDÁVEL

//Parte 3.1- Simular um coração saudável
	
	//A freqûenca cardíaca do paciente deve ser constantemente monitorada, para que seja possível estabelecer o fluxo desejado da bomba. Ela pode ser calculada por meio
	//de uma função senoidal (pq essa função começa em 0 e vai crescendo), em que F(t) = A * sen (2pi *f* t ) + B, onde F(t) é a frequência ao longo do tempodeamostragem, 
	//A é a amplitude da onda, 2pi aparece na equação porque uma função seno gera um ciclo completo de 360º, f é a frequência que o médico digitou inicialmente,
	//t é o tempodeamostragem em segundos e B é onde começa o batimento.
	
	//Graficamente, no eixo X tera o tempo de amostragem e no Y, a pressão de enchimento do coração na sístole e diástole.
	//Um coração normal possue uma pressão de 130mmHg durante a sístole e 85mmHg na diástole, logo A = 130 - 85 / 2 = 22,5. (divide por 2 porque há valores encima e embaixo da onda).
	//B = 130+85/2 = 107,5 (divide por 2 porque há valores encima e embaixo). Em atividade as pressão de sísitole e diástole variam de 220mmHg (sístole) e 60mmHg (diástole),
	//logo B = 220 + 60 / 2 = 140 e A = 220-60 / 2 = 80. Em repouso, a pressão durante a sístole e diástole é em média 120mmHg (sístole) e 80mmHg(diástole).
	//Logo a = 120-80/2 = 20 e B = 120+80/2 = 100
	
	// t = 2pi/100, porque eu quero que a leitura seja realizada em 100intervalos (eu escolhi esse valor
	//para que a visualização do código fosse mais simples, mas na vida real teriam que ser intervalos bem menores). 
	//T precisa ser um vetor, pois eu quero a freq monitorada ao longo e vários pontos.
	//If x > 0 foi colocado para o sistema calcular constantemente a frequencia da bomba, já que ela nunca assumirá um valor negativo
	
	//Com relação a onda fe fluxo por tempo, F(t) = A * sen (2pi *f* t) + B, onde F(t) é a frequência ao longo do tempodeamostragem, 
	//A é a amplitude da onda, 2pi aparece na equação porque uma função seno gera um ciclo completo de 360º, f é a frequência que o médico digitou inicialmente,
	//t é o tempodeamostragem em segundos e B é onde começa o batimento.
	
	//Graficamente, no eixo X tera o tempodeamostragem e no Y o fluxo sanguíneo do coração na sístole e diástole.	
	//fluxo médio de um paciente normal --> A = 1 e B = 6,10 (débito cardíaco igual a 7,2 litros por segundo)
	//fluxo médio de um paciente em repouso --> B = 4,5 e A = 1 (débito cardíaco igual a a 3,73 litros por segundo)
	//fluxo médio de um paciente em atividade --> A = 4 e B = 11,1 (débito caríaco igual a 15.6 litros por segundo)
	
//Parte 3.2 - Simular um coração com IC
	
	//ONDA DE PRESSÃO POR TEMPO DE AMOSTRAGEM: 
		//A lógica aplicada é a mesma para o coração normal, a diferença é que A e B terão os valores de um coração com insuficiência cardíaca:
		//A pressão arterial de um coração com insuficência caríaca varia entre 140 e 80mmHg. Logo: A = 140-80 / 2 = 30. B é = 110.
		//o fluxo sanguíneo dentro de um coração que sofre de IC é de 2,2l/seg. Logo A = 1.1 e B =1.1
		
		//printf("\n\nVARIAÇÃO DA PRESSÃO E FLUXO SANGUÍNEO NO tempodeamostragem, EM UM CORAÇÃO COM IC: \n\n");
		//printf("\n\nVARIAÇÃO DO FLUXO NO tempodeamostragem, EM UM CORAÇÃO COM IC: \n\n");
		
//Parte 3.3 - Determinar o estado de resistência do paciente, com base em avlores reais.
	
			//printf("VARIAÇÃO DA FREQUÊNCIA E FLUXO NO tempodeamostragem: \n\n");
				for (int i = 0; i <= 100; i++){
							tempodeamostragem[i] = i * 0.0628;
							pressaosimuladaIC[i] =  30 * sin(6.28 * freqseg * tempodeamostragem[i]) + 110;
							fluxosimuladoIC[i] =  1.1 * sin(6.28 * freqseg * tempodeamostragem[i]) + 1.1;
							
								if (estadocardiacodopaciente < 60){
								//essa condição foi imposta, pois um coração, em repouso, tem uma freqûencia de em média 60batimentos por minuto
								pressaosimulada[i] =  20 * sin(6.28 * freqseg * tempodeamostragem[i]) + 100;
								//Os valores de fluxo variam entre 4, 5 e 6 litros
								fluxosimulado[i] = 1 * sin(6.28 * freqseg * tempodeamostragem[i]) + 4.5;
								//printf("Para fluxo = %.4lf seg, pressão monitorada = %.4lf\n", fluxosimulado[i],pressaosimulada[i]);
								//printf("Para tempodeamostragem = %.4lf seg, pressão simulada = %.4lf\n", tempodeamostragem [i], pressaosimulada[i]);
								estadoderesistencia = 125;
							}	
								else if ((estadocardiacodopaciente >= 60)&& (estadocardiacodopaciente < 100)){
								//essa condição foi imposta, pois um coração, em estado normal, tem uma frequência entre 100 e 60 batimentos por min.
								pressaosimulada[i] = 22.5 * sin(6.28 * freqseg * tempodeamostragem[i]) + 107.5;
								//os valores de fluxo variam entre 6, 7 e 8
								fluxosimulado[i] = 1 * sin(6.28 * freqseg * tempodeamostragem[i]) + 6.1;
								//printf("Para fluxo = %.4lf seg, pressão simulada = %.4lf\n", fluxosimulado[i],pressaosimulada[i]);
								//printf("Para tempodeamostragem = %.4lf seg, pressão simulada = %.4lf\n", tempodeamostragem [i],pressaosimulada[i]);
								estadoderesistencia = 250;
							}
								else{
								//essa condição foi imposta, pois um coração, em atividade física, tem uma freqûencia entre 100 e 133 batimentos por min.
								pressaosimulada[i] =  80 * sin(6.28 * freqseg * tempodeamostragem[i]) + 140;
								// o fluxo varia entre 14, 15 e 16 litros
								fluxosimulado[i] =  4 * sin(6.28 * freqseg * tempodeamostragem[i]) + 11.1;
								//printf("Para fluxo = %.4lf seg, pressão simulada = %.4lf\n", fluxosimulado[i],pressaosimulada[i]);
								//printf("Para tempodeamostragem = %.4lf seg, pressão simulada = %.4lf\n", tempodeamostragem [i],pressaosimulada[i]);
								estadoderesistencia = 500;
							}
							

			
	//PARTE 4 - CRIAR OS ESTIMADORES:
		//ESTIMADOR DE FREQUÊNCIA CARDÍACA
 			
			for(int i=0; i <=31; i++){
				//i vai de 0 até 31, pois em um intervalo de 2segundos, tem-se 32 pontos no gráfico da função seno. Os 2 segundos foram escolhidos porque existem pacientes com batimentos de até 30batimentos por minuto... 
				//Dessa forma, em 2 segundos é possível descrever esta onda completa.
				if (pressaosimulada[i] > pressaosimulada[i-1]){
					picomaximofcard = pressaosimulada[i];
				}
				else{
  						contadordepicofcard++;
						picomaximofcard = -1000;
					}	
				}
			}
		printf("Frequência cardíaca estimada: %d\n", contadordepicofcard*30);
		
		//ESTIMADOR DE FLUXO P2P REAL
			for(int i = 0; i <= 31; i++){
				fluxoreal[i] = fluxosimulado[i] - fluxosimuladoIC[i];
				if(fluxoreal[i] > fluxoreal[i-1]){
				picomaximoflu = fluxoreal[i];
				}
			
			else{
				picominimoflu = fluxoreal[i];
				}
			}
			
			printf("\nO pico máximo é: %.4f\n", picomaximoflu);
			printf("\nO pico mínimo é: %.4f\n", picominimoflu);
			P2Preal = picomaximoflu - picominimoflu;
			printf("\nO P2Preal é: %.4f\n", P2Preal);
			
		//ESTIMADOR DE FLUXO DESEJADO:
	
			//O fluxo desejado da bomba é descrito por uma reta, retratada pela  figura 1 do artigo de Vollkron. Na figura é possível obersar 2 pontos, pela equação da reta:
			// Y = coeficientedareta(x - xa) + ya e coeficientedareta = (yb - ya) / (xb - xa). Considerando Xb = freq cardiaca em repouso, xa= freq cardiaca em exercício, 
			//ya = fluxo em exercício, yb = fluxo em repouso. Tem-se:
			coeficientedareta = (fluxocoracaoexercicio - fluxocoracaoemrepouso)/(freqcardexercicio - freqcardrepouso);
			fluxodesejado = (coeficientedareta *(estadocardiacodopaciente - freqcardrepouso)) + fluxocoracaoemrepouso;
			printf ("\nINFORMAÇÕES: \nValor de fluxo P2P: %.3f\nPotência máxima admissível: %.3f\nFluxo mínimo: %.3f\nFluxo desejado: %.3fl/min\n", P2Pdesejado,Potmaxdesejada,fluxocoracaoemrepouso,fluxodesejado);
			
		//PARTE 4 - SIMULAR O CONTROLADOR:
		
		//A pressão real da bomba é o quanto um coração normal fornece, menos oque o coração com Ic fornece, o mesmo acontece com o fluxo.	
		//Para achar a resistência faz-se uma regra de 3. 4000 rotações por minuito está para 10litros, e Xrotações está para o estadodefluxodabomba.
		//Como a rotação máxima do DAV é de 4000 rotações por minuto e isso corresponde a 2A, faz-se uma regra de 3 em que a rotação atual corresponde a variação da corrente no fluxo.
	
			//*BONS VALORES PARA GANHO*//
			//Xp2p varia entre 1.5 e 2.0, como o fluxo P2P não está funcionanendo perfeitamente, não é possível estabelecer um ganho que sirva para todos os casos
			//Rotação do dav é entre 6000 e 5000 rpm. Com isso, considerand que valhe 6000 extraí-se um calor de Xpot de aproximadamente 2400 então um bom ganho seria de:
			//Xfluxo é de até 10l/min, logo K1 pode ser igual a 7, pois os valores dão entorno de 5l/min para as condições
			
			printf("\nINFORMAÇÕES\n");
			
			for (int i=0; i<=100; i++){
				
				pressaoreal[i] = pressaosimuladaIC[i] - pressaosimulada[i];
				fluxoreal[i] = fluxosimulado[i] - fluxosimuladoIC[i]+Xfluxovelocidadedabomba; 
				rotacaoreal = (4000 * fluxoreal[i])/10;
				rotacaoreal = rotacaoreal - estadoderesistencia;
				correntereal = (2 * rotacaoreal)/4000;
				Potreal = correntereal * 24;
				
				if(fluxoreal[i] > fluxoreal[i-1]){
				picomaximoflu = fluxoreal[i];
				}
			else{
				picominimoflu = fluxoreal[i];
				}
				
			if (i%31 == 0){
				picomaximoflu = -1000;
				picominimoflu = 1000;
				}
			
			if (picomaximoflu > -100 && picominimoflu < 100){
			//printf("\nO pico máximo é: %.4f\n", picomaximoflu);
			//printf("\nO pico mínimo é: %.4f\n", picominimoflu);
			P2Preal = picomaximoflu - picominimoflu; }


				Xfluxo[i] = (fluxodesejado - fluxoreal[i]) * kp1;
				Xpot= (Potmaxdesejada - Potreal) * kp2; //kp2;
				XP2P = (P2Preal - P2Pdesejado) * kp3; //kp3;
				tempodeamostragem[i] = i * 0.0628;
				printf("Em %.4lf seg: Xfluxo(t) = %.2f, Xpot(t) = %.2f, XP2P(t) = %.2f\n",tempodeamostragem[i], Xfluxo[i], Xpot, XP2P);
				//dentre estas 3 variáveis, o sistema salva como critério o menor valor:
			}
			
			for(int i=0; i <= 100; i++){
				if(Xfluxo[i]<Xpot && Xfluxo[i]<XP2P){
					Xseccao1 = Xfluxo[i];
					//printf("O menor valor é: %.2f\n",criterio1);
				}
			else if(Xpot<Xfluxo[i] && Xpot<XP2P){
					Xseccao1 = Xpot;
					//printf("O menor valor é: %.2f\n",criterio1);
				}
			else {
					Xseccao1 = XP2P;
					//printf("O menor valor é: %.2f\n",criterio1);
				}
				
			Xfluxominimo[i] = (fluxoreal[i] - fluxocoracaoemrepouso) * kp4; //kp4;

			if(Xfluxominimo[i] > Xseccao1){
					Xvelocidade = Xfluxominimo[i];
			}
			else{
					Xvelocidade = Xseccao1;
				}
				//o ajuste é da velocidade da bomba
				Xfluxovelocidadedabomba = (Xvelocidade*10)/4000;
				printf("O maior valor entre %.2f e %.2f é: %.2f com fluxo de %.2f\n", Xseccao1, Xfluxominimo[i], Xvelocidade, Xfluxovelocidadedabomba);
				
			}
}
