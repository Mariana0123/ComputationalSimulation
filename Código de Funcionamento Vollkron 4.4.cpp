// CONTROLADOR DE MALHA FECHADA
# include <stdio.h>
# include <locale.h>
# include <stdlib.h>
# include <math.h>

int main(){
	
	setlocale(LC_ALL, "Portuguese");
	
//PARTE 1 - DECLARA��O DAS VARI�VEIS
	
	//Equa��o da reta afim do fluxo ideal:
	float freqcardrepouso, freqcardexercicio, fluxocoracaoemrepouso, fluxocoracaoexercicio, coeficientedareta; 
	//Valores desejados de controle:
	float Potmaxdesejada, fluxodesejado, P2Pdesejado;
	//Estados iniciais do paciente simulado:
	float estadocardiacodopaciente, estadodefluxodabomba, estadoderesistencia = 0;
	//Vari�veis utilizadas para simular o paciente:	
	float freqseg, tempodeamostragem[1000], pressaosimulada[1000], fluxosimulado[1000],pressaosimuladaIC[1000], fluxosimuladoIC[1000];
	//Vari�veis de realimenta��o do controle:
	float pressaoreal[1000], fluxoreal[1000], Potreal, P2Preal, rotacaoreal, correntereal; 
	//Vari�veis de estimadores do paciente:
	int picomaximofcard = -1000, contadordepicofcard = 0;
	float picomaximoflu = 0, picominimoflu = 0, constantederotacao = 0;
	//Ganhos do controle de velocidade:
	float kp1 = 400, kp2 = 83.3, kp3 = 600, kp4 = 800;
	//Sa�das do controle:
	float Xpot, XP2P, Xfluxo[1000], Xfluxominimo[1000], Xvelocidade, Xseccao1, Xfluxovelocidadedabomba = 0;

//PARTE 2 - SOLICITAR AS VARI�VEIS DE ENTRADA

	printf("M�dico, entre com a frequ�ncia card�aca durante o repouso: \n(exemplo: 30)\n");
	scanf("%f", &freqcardrepouso);
	printf("\nEntre com a frequ�ncia card�aca durante o exerc�cio: \n(exemplo: 120)\n");
	scanf("%f", &freqcardexercicio);
	printf("\nEntre com o fluxo em exerc�cio: \n(exemplo: 10)\n");
	scanf("%f", &fluxocoracaoexercicio);
	printf("\nEntre com o valor de fluxo em repouso: \n(exemplo: 3)\n");
	scanf("%f", &fluxocoracaoemrepouso);
	printf("\nEntre com o estado de fluxo da bomba: \n(exemplo: 4)\n");
	scanf("%f", &estadodefluxodabomba);
	rotacaoreal = (4000 * estadodefluxodabomba)/10;
	printf("\nEntre com o estado card�aco do paciente: \n(exemplo: 60)\n");
	scanf("%f", &estadocardiacodopaciente);
	freqseg = estadocardiacodopaciente/60;
	//a pot�ncia m�xima da bomba corresponde ao maior valor que a bomba pode consumir de energia.
	printf("\nEntre com a pot�ncia m�xima desejada da bomba: \n(exemplo: 48)\n");
	scanf("%f", &Potmaxdesejada);
	printf("\nEntre com o fluxo P2P desejado da bomba: \n(exemplo: 2)\n");
	scanf("%f", &P2Pdesejado);
	printf("\n");
	
//PARTE 3 - SIMULAR UM CORA��O SAUD�VEL

//Parte 3.1- Simular um cora��o saud�vel
	
	//A freq�enca card�aca do paciente deve ser constantemente monitorada, para que seja poss�vel estabelecer o fluxo desejado da bomba. Ela pode ser calculada por meio
	//de uma fun��o senoidal (pq essa fun��o come�a em 0 e vai crescendo), em que F(t) = A * sen (2pi *f* t ) + B, onde F(t) � a frequ�ncia ao longo do tempodeamostragem, 
	//A � a amplitude da onda, 2pi aparece na equa��o porque uma fun��o seno gera um ciclo completo de 360�, f � a frequ�ncia que o m�dico digitou inicialmente,
	//t � o tempodeamostragem em segundos e B � onde come�a o batimento.
	
	//Graficamente, no eixo X tera o tempo de amostragem e no Y, a press�o de enchimento do cora��o na s�stole e di�stole.
	//Um cora��o normal possue uma press�o de 130mmHg durante a s�stole e 85mmHg na di�stole, logo A = 130 - 85 / 2 = 22,5. (divide por 2 porque h� valores encima e embaixo da onda).
	//B = 130+85/2 = 107,5 (divide por 2 porque h� valores encima e embaixo). Em atividade as press�o de s�sitole e di�stole variam de 220mmHg (s�stole) e 60mmHg (di�stole),
	//logo B = 220 + 60 / 2 = 140 e A = 220-60 / 2 = 80. Em repouso, a press�o durante a s�stole e di�stole � em m�dia 120mmHg (s�stole) e 80mmHg(di�stole).
	//Logo a = 120-80/2 = 20 e B = 120+80/2 = 100
	
	// t = 2pi/100, porque eu quero que a leitura seja realizada em 100intervalos (eu escolhi esse valor
	//para que a visualiza��o do c�digo fosse mais simples, mas na vida real teriam que ser intervalos bem menores). 
	//T precisa ser um vetor, pois eu quero a freq monitorada ao longo e v�rios pontos.
	//If x > 0 foi colocado para o sistema calcular constantemente a frequencia da bomba, j� que ela nunca assumir� um valor negativo
	
	//Com rela��o a onda fe fluxo por tempo, F(t) = A * sen (2pi *f* t) + B, onde F(t) � a frequ�ncia ao longo do tempodeamostragem, 
	//A � a amplitude da onda, 2pi aparece na equa��o porque uma fun��o seno gera um ciclo completo de 360�, f � a frequ�ncia que o m�dico digitou inicialmente,
	//t � o tempodeamostragem em segundos e B � onde come�a o batimento.
	
	//Graficamente, no eixo X tera o tempodeamostragem e no Y o fluxo sangu�neo do cora��o na s�stole e di�stole.	
	//fluxo m�dio de um paciente normal --> A = 1 e B = 6,10 (d�bito card�aco igual a 7,2 litros por segundo)
	//fluxo m�dio de um paciente em repouso --> B = 4,5 e A = 1 (d�bito card�aco igual a a 3,73 litros por segundo)
	//fluxo m�dio de um paciente em atividade --> A = 4 e B = 11,1 (d�bito car�aco igual a 15.6 litros por segundo)
	
//Parte 3.2 - Simular um cora��o com IC
	
	//ONDA DE PRESS�O POR TEMPO DE AMOSTRAGEM: 
		//A l�gica aplicada � a mesma para o cora��o normal, a diferen�a � que A e B ter�o os valores de um cora��o com insufici�ncia card�aca:
		//A press�o arterial de um cora��o com insufic�ncia car�aca varia entre 140 e 80mmHg. Logo: A = 140-80 / 2 = 30. B � = 110.
		//o fluxo sangu�neo dentro de um cora��o que sofre de IC � de 2,2l/seg. Logo A = 1.1 e B =1.1
		
		//printf("\n\nVARIA��O DA PRESS�O E FLUXO SANGU�NEO NO tempodeamostragem, EM UM CORA��O COM IC: \n\n");
		//printf("\n\nVARIA��O DO FLUXO NO tempodeamostragem, EM UM CORA��O COM IC: \n\n");
		
//Parte 3.3 - Determinar o estado de resist�ncia do paciente, com base em avlores reais.
	
			//printf("VARIA��O DA FREQU�NCIA E FLUXO NO tempodeamostragem: \n\n");
				for (int i = 0; i <= 100; i++){
							tempodeamostragem[i] = i * 0.0628;
							pressaosimuladaIC[i] =  30 * sin(6.28 * freqseg * tempodeamostragem[i]) + 110;
							fluxosimuladoIC[i] =  1.1 * sin(6.28 * freqseg * tempodeamostragem[i]) + 1.1;
							
								if (estadocardiacodopaciente < 60){
								//essa condi��o foi imposta, pois um cora��o, em repouso, tem uma freq�encia de em m�dia 60batimentos por minuto
								pressaosimulada[i] =  20 * sin(6.28 * freqseg * tempodeamostragem[i]) + 100;
								//Os valores de fluxo variam entre 4, 5 e 6 litros
								fluxosimulado[i] = 1 * sin(6.28 * freqseg * tempodeamostragem[i]) + 4.5;
								//printf("Para fluxo = %.4lf seg, press�o monitorada = %.4lf\n", fluxosimulado[i],pressaosimulada[i]);
								//printf("Para tempodeamostragem = %.4lf seg, press�o simulada = %.4lf\n", tempodeamostragem [i], pressaosimulada[i]);
								estadoderesistencia = 125;
							}	
								else if ((estadocardiacodopaciente >= 60)&& (estadocardiacodopaciente < 100)){
								//essa condi��o foi imposta, pois um cora��o, em estado normal, tem uma frequ�ncia entre 100 e 60 batimentos por min.
								pressaosimulada[i] = 22.5 * sin(6.28 * freqseg * tempodeamostragem[i]) + 107.5;
								//os valores de fluxo variam entre 6, 7 e 8
								fluxosimulado[i] = 1 * sin(6.28 * freqseg * tempodeamostragem[i]) + 6.1;
								//printf("Para fluxo = %.4lf seg, press�o simulada = %.4lf\n", fluxosimulado[i],pressaosimulada[i]);
								//printf("Para tempodeamostragem = %.4lf seg, press�o simulada = %.4lf\n", tempodeamostragem [i],pressaosimulada[i]);
								estadoderesistencia = 250;
							}
								else{
								//essa condi��o foi imposta, pois um cora��o, em atividade f�sica, tem uma freq�encia entre 100 e 133 batimentos por min.
								pressaosimulada[i] =  80 * sin(6.28 * freqseg * tempodeamostragem[i]) + 140;
								// o fluxo varia entre 14, 15 e 16 litros
								fluxosimulado[i] =  4 * sin(6.28 * freqseg * tempodeamostragem[i]) + 11.1;
								//printf("Para fluxo = %.4lf seg, press�o simulada = %.4lf\n", fluxosimulado[i],pressaosimulada[i]);
								//printf("Para tempodeamostragem = %.4lf seg, press�o simulada = %.4lf\n", tempodeamostragem [i],pressaosimulada[i]);
								estadoderesistencia = 500;
							}
							

			
	//PARTE 4 - CRIAR OS ESTIMADORES:
		//ESTIMADOR DE FREQU�NCIA CARD�ACA
 			
			for(int i=0; i <=31; i++){
				//i vai de 0 at� 31, pois em um intervalo de 2segundos, tem-se 32 pontos no gr�fico da fun��o seno. Os 2 segundos foram escolhidos porque existem pacientes com batimentos de at� 30batimentos por minuto... 
				//Dessa forma, em 2 segundos � poss�vel descrever esta onda completa.
				if (pressaosimulada[i] > pressaosimulada[i-1]){
					picomaximofcard = pressaosimulada[i];
				}
				else{
  						contadordepicofcard++;
						picomaximofcard = -1000;
					}	
				}
			}
		printf("Frequ�ncia card�aca estimada: %d\n", contadordepicofcard*30);
		
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
			
			printf("\nO pico m�ximo �: %.4f\n", picomaximoflu);
			printf("\nO pico m�nimo �: %.4f\n", picominimoflu);
			P2Preal = picomaximoflu - picominimoflu;
			printf("\nO P2Preal �: %.4f\n", P2Preal);
			
		//ESTIMADOR DE FLUXO DESEJADO:
	
			//O fluxo desejado da bomba � descrito por uma reta, retratada pela  figura 1 do artigo de Vollkron. Na figura � poss�vel obersar 2 pontos, pela equa��o da reta:
			// Y = coeficientedareta(x - xa) + ya e coeficientedareta = (yb - ya) / (xb - xa). Considerando Xb = freq cardiaca em repouso, xa= freq cardiaca em exerc�cio, 
			//ya = fluxo em exerc�cio, yb = fluxo em repouso. Tem-se:
			coeficientedareta = (fluxocoracaoexercicio - fluxocoracaoemrepouso)/(freqcardexercicio - freqcardrepouso);
			fluxodesejado = (coeficientedareta *(estadocardiacodopaciente - freqcardrepouso)) + fluxocoracaoemrepouso;
			printf ("\nINFORMA��ES: \nValor de fluxo P2P: %.3f\nPot�ncia m�xima admiss�vel: %.3f\nFluxo m�nimo: %.3f\nFluxo desejado: %.3fl/min\n", P2Pdesejado,Potmaxdesejada,fluxocoracaoemrepouso,fluxodesejado);
			
		//PARTE 4 - SIMULAR O CONTROLADOR:
		
		//A press�o real da bomba � o quanto um cora��o normal fornece, menos oque o cora��o com Ic fornece, o mesmo acontece com o fluxo.	
		//Para achar a resist�ncia faz-se uma regra de 3. 4000 rota��es por minuito est� para 10litros, e Xrota��es est� para o estadodefluxodabomba.
		//Como a rota��o m�xima do DAV � de 4000 rota��es por minuto e isso corresponde a 2A, faz-se uma regra de 3 em que a rota��o atual corresponde a varia��o da corrente no fluxo.
	
			//*BONS VALORES PARA GANHO*//
			//Xp2p varia entre 1.5 e 2.0, como o fluxo P2P n�o est� funcionanendo perfeitamente, n�o � poss�vel estabelecer um ganho que sirva para todos os casos
			//Rota��o do dav � entre 6000 e 5000 rpm. Com isso, considerand que valhe 6000 extra�-se um calor de Xpot de aproximadamente 2400 ent�o um bom ganho seria de:
			//Xfluxo � de at� 10l/min, logo K1 pode ser igual a 7, pois os valores d�o entorno de 5l/min para as condi��es
			
			printf("\nINFORMA��ES\n");
			
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
			//printf("\nO pico m�ximo �: %.4f\n", picomaximoflu);
			//printf("\nO pico m�nimo �: %.4f\n", picominimoflu);
			P2Preal = picomaximoflu - picominimoflu; }


				Xfluxo[i] = (fluxodesejado - fluxoreal[i]) * kp1;
				Xpot= (Potmaxdesejada - Potreal) * kp2; //kp2;
				XP2P = (P2Preal - P2Pdesejado) * kp3; //kp3;
				tempodeamostragem[i] = i * 0.0628;
				printf("Em %.4lf seg: Xfluxo(t) = %.2f, Xpot(t) = %.2f, XP2P(t) = %.2f\n",tempodeamostragem[i], Xfluxo[i], Xpot, XP2P);
				//dentre estas 3 vari�veis, o sistema salva como crit�rio o menor valor:
			}
			
			for(int i=0; i <= 100; i++){
				if(Xfluxo[i]<Xpot && Xfluxo[i]<XP2P){
					Xseccao1 = Xfluxo[i];
					//printf("O menor valor �: %.2f\n",criterio1);
				}
			else if(Xpot<Xfluxo[i] && Xpot<XP2P){
					Xseccao1 = Xpot;
					//printf("O menor valor �: %.2f\n",criterio1);
				}
			else {
					Xseccao1 = XP2P;
					//printf("O menor valor �: %.2f\n",criterio1);
				}
				
			Xfluxominimo[i] = (fluxoreal[i] - fluxocoracaoemrepouso) * kp4; //kp4;

			if(Xfluxominimo[i] > Xseccao1){
					Xvelocidade = Xfluxominimo[i];
			}
			else{
					Xvelocidade = Xseccao1;
				}
				//o ajuste � da velocidade da bomba
				Xfluxovelocidadedabomba = (Xvelocidade*10)/4000;
				printf("O maior valor entre %.2f e %.2f �: %.2f com fluxo de %.2f\n", Xseccao1, Xfluxominimo[i], Xvelocidade, Xfluxovelocidadedabomba);
				
			}
}
