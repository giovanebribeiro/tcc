/**
 * @file gprs.h   
 * @author Giovane Boaviagem Ribeiro   
 *                                                      
 * @brief Arquivo de header da API de comunica��o GPRS entre microcontroladores PIC. 
 * @details 
 * 
 * Este arquivo se divide em quatro partes, a saber
 * <ul>
 *    <li>Fun��es auxiliares (Fun��es que ajudam na implementa��o das fun��es principais)</li>
 *    <li>Fun��es de inicializa��o (Respons�veis por carregar as vari�veis globais com os valores iniciais)</li>
 *    <li>Fun��es de verifica��o</li>
 *    <li>Fun��es de envio/recebimento</li>
 * </ul>
 * 
 * 
 * Pr�-condi��es de uso:
 * <ul>
 *    <li> O modem a ser utilizado deve ser compat�vel com comandos AT de especifica��es GSM 07.05 e 07.07</li>
 *    <li> A liga��o entre o modem e o PIC � realizada por meio de porta serial (por hardware).</li>
 * </ul>  
 *
 */
 
#include <stdlib.h>
#include <string.h>

#ifndef _GPRS_H
#define _GPRS_H


/*********************************************************
 * Vari�veis globais da biblioteca, e constantes #define *
 *********************************************************/

#define MAX_BUFFER 99
#define MED_BUFFER 45
#define MIN_BUFFER 20
#define U_BUFFER 10 

   char bufferSerial[MAX_BUFFER];//armazenar� os valores da porta serial.
   int indiceBuffer;//indice do array de resposta.
   char *destino;//n�mero de telefone para onde ser�o enviados as mensagens de SMS
   
/**********************
 * Fun��es auxiliares *
 **********************/
 
 /**
  * Limpa o buffer
  */
   void limparBuffer(){      
      for(indiceBuffer=0;indiceBuffer<MAX_BUFFER;indiceBuffer++){
         bufferSerial[indiceBuffer]='0';
      }      
   }
   
   /**
    * Carrega o buffer com algum valor presente na porta serial.
    */
   void carregarBuffer(){      
      int temp=0;
      indiceBuffer=0;
      while(kbhit()){
         temp=getc(); //pega o valor na porta serial.
         bufferSerial[indiceBuffer]=temp;
         indiceBuffer=(indiceBuffer+1)%MAX_BUFFER; //incremento circular.
      }
   }
   
   /**
    * Retorna o �ndice da primeira ocorr�ncia do caractere na string
    * @param string String a ser mapeada
    * @param c caractere a ser encontrado na string passada
    * @return O �ndice do caractere passado como par�metro na string tamb�m passada.
    */
   int indexOf(char *string, char c){
     int i;
     char a;
     int ret;
     
     ret=0;
     
     for(i=0;i<strlen(string);i++){
         a=*(string+i);
         if(a==c){
            ret=i;
            break;
         }
     }
     
     return ret;
   }
   
   /**
    * Retorna uma parte da string original, cujos limites s�o definidos 
    * pelos valores passados como par�metro.
    * @param str String a ser extra�da a substring
    * @param limiteInicial Limite inicial da substring
    * @param limiteFinal Limite final da substring
    * @return A string entre os limites inicial e final.
    */
   char *substring(char *str,int limiteInicial,int limiteFinal){
      char *res;
      int i;
      int tamanho;
      
      tamanho=strlen(str);
      
      res=str;
      
      strcpy(res,str);     
      
      if(limiteInicial<0 || limiteInicial>tamanho){
         limiteInicial=0;
      }
      
      if(limiteFinal>=tamanho){
         limiteFinal=tamanho-1;
      }
      
      for(i=limiteInicial;i<=limiteFinal;i++){
         *(res+i)=*(str+i);
      }
      
      //marca o fim da string
      *(res+i)='\0';
      
      return res;
   }
   
   /**
    * @return A quantidade de cr�ditos do SIM card.
    */
   float getCreditos(void);


/***************************
 * Fun��o de inicializa��o *
 ***************************/

   /**
    * Rotina que inicializa a biblioteca GPRS.
    * @param dest N�mero do telefone para onde o modem deve enviar as mensagens. 
    * Ele deve estar no seguinte formato: +55xxyyyyyyyy, onde xx � o c�digo da 
    * operadora, e yyyyyyyy � o dispositivo m�vel escolhido.
    */
   void gprsInit(char *dest){
      indiceBuffer=0;
      limparBuffer();
      destino=dest;
   }  


/**************************
 * Fun��es de Verifica��o *
 **************************/

   /**
    * Verifica se existe algum modem conectado ao microcontrolador
    * @return 0, se n�o houver modem, ou 1, em caso contr�rio
    */
   short existeModem(void){
      limparBuffer(); //limpa o buffer antes da execu��o da rotina
      printf("AT");
      putc(0x0d);//CR
      putc(0x0a);//Nova linha      
      
      carregarBuffer();
      
      delay_ms(500);
      
      if(bufferSerial[0]=='O'&& bufferSerial[1]=='K'){ 
         return 1;
      }else{
         return 0;
      }     
   }

   /**
    * Verifica se existe sinal para transmis�o do dado.
    * @return Codigos de saida descritos abaixo:
    * <ul>
    *    <li>-1: N�o h� conex�o com o modem.</li>
    *    <li>0: Nenhum sinal.</li>
    *    <li>1: Sinal fraco.</li>
    *    <li>2: Sinal regular.</li>
    *    <li>3: Sinal bom.</li>
    * </ul>
    */
   int existeSinal(void){      
      //array que armazenar� a sa�da do m�todo subArrayInt.
      int limiteInicial;
      int limiteFinal;
      int ret;
      
      char *qualidade;
      char *semConexao="99";
      char *nenhumSinal="0";
      char *sinalFraco="1";
      char *sinalRegularInferior="2";
      char *sinalRegularSuperior="30";
      char *sinalBom="31";
      
      ret=0;
      
      limparBuffer();
      printf("AT+CSQ");
      putc(0x0d);//CR
      putc(0x0a);//Nova linha
      //Exemplo de saida esperada: +CSQ: 21,99
      
      carregarBuffer();
      /*Aqui, n�s temos:
      resp[0]='+'
      resp[1]='C'
      resp[2]='S'
      ...
      */
      delay_ms(500);
      
      limiteInicial=indexOf(bufferSerial,' ');
      
      delay_ms(500);

      limiteFinal=indexOf(bufferSerial,',');
      
      delay_ms(500);
      
      qualidade=substring(bufferSerial,limiteInicial,limiteFinal);
      
      delay_ms(500);
      
      //verificando a qualidade do sinal
      if(strcmp(qualidade,nenhumSinal)){
         ret=0;
      }else if(strcmp(qualidade,sinalFraco)){
         ret=1;
      }else if(strcmp(qualidade,sinalRegularInferior)>0 && strcmp(qualidade,sinalRegularSuperior)<0){
         ret=2;
      }else if(strcmp(qualidade,sinalBom)>0){
         ret=3;
      }else if(strcmp(qualidade,semConexao)){
         ret=-1;
      }
      
      return ret;
   }

   /**
    * Verifica  se existe cr�ditos suficientes no chip para o envio da mensagem
    * @return 0, se n�o existir cr�ditos suficientes, ou 1, em caso contr�rio   
    */
   short existeCredito(void);
   
   
/********************************
 * Fun��es de envio/recebimento *
 ********************************/

   /**
    * Envia uma string pela rede
    * @param str A string a ser enviada.
    * @return 0, se a opera��o foi mal sucedida, ou 1, em caso contr�rio 
    */
   short enviaString(char *str){
      //short resp;
      
      limparBuffer();
      printf("AT+CMGF=1");
      putc(0x0d);//CR
      putc(0x0a);//Nova linha
      //sa�da esperada: OK
      
      carregarBuffer();
      
      delay_ms(500);
      
      if(bufferSerial[0]=='O'&& bufferSerial[1]=='K'){ 
         limparBuffer();
         printf("AT+CMGS=\"%s\"",destino);
         putc(0x0d);//CR
         putc(0x0a);//Nova linha
         printf("%s",str);
         putc(0x1a); //equivalente ao ctrl+z
         carregarBuffer();
         if(bufferSerial[0]=='O'&& bufferSerial[1]=='K'){
            return 1;
         }else{
            return 0;
         }
      }else{
         return 0;
      }     
   }

   /**
    * Recebe um string pela rede
    * @return A string recebida
    */
   char *recebeString(void){
      char *str;
      char *aux;
      char *aux2;
      char *idMensagem;
      int temp;
      int temp2;
      int i;
      
      
      limparBuffer();
      aux="+CMTI:\"SM";
      temp=0;
      temp2=0;
      
      //la�o que verifica se chegou mensagem
      while(1){
         carregarBuffer();
         temp=indexOf(bufferSerial, ',');
         temp--;
         aux2=substring(bufferSerial,0,temp);
         if(strcmp(aux2,aux)){
            temp2=strlen(bufferSerial);
            idMensagem=substring(bufferSerial,temp,temp2);
            break;
         }
         delay_ms(500);
      }
      
      limparBuffer();
      printf("AT+CMGR=%s",idMensagem);
      carregarBuffer();
      
      //com isso, aux ficar� com ("06/11/11,00:32:20+32" A simple demo of SMS text messaging.) ,por exemplo
      temp=indexOf(bufferSerial,',');
      temp2=strlen(bufferSerial);
      aux=substring(bufferSerial,temp+1,temp2);
      for(i=0;i<2;i++){
         temp=indexOf(aux,',');
         temp2=strlen(aux);
         aux=substring(aux,temp+1,temp2);
      }
      
      //resultado: 32" A simple demo of SMS text messaging.
      temp=indexOf(aux,'+');
      temp2=strlen(aux);
      aux=substring(aux,temp+1,temp2);
      
      temp=indexOf(aux,'"');
      temp2=strlen(aux);
      aux=substring(aux,temp+1,temp2);
      
      
      strcpy(str,aux); 
      return str;      
   }

   /**
    * Envia um n�mero inteiro pela rede
    * @param x O n�mero a ser enviado
    * @return 0, se a opera��o foi mal sucedida, ou 1, em caso contr�rio 
    */
   short enviaInt(int x){
      //short resp;
      
      limparBuffer();
      printf("AT+CMGF=1");
      putc(0x0d);//CR
      putc(0x0a);//Nova linha
      //sa�da esperada: OK
      
      carregarBuffer();
      
      delay_ms(500);
      
      if(bufferSerial[0]=='O'&& bufferSerial[1]=='K'){ 
         limparBuffer();
         printf("AT+CMGS=\"%s\"",destino);
         putc(0x0d);//CR
         putc(0x0a);//Nova linha
         printf("%d",x);
         putc(0x1a); //equivalente ao ctrl+z
         carregarBuffer();
         if(bufferSerial[0]=='O'&& bufferSerial[1]=='K'){
            return 1;
         }else{
            return 0;
         }
      }else{
         return 0;
      }     
   }

   /**
    * Recebe um n�mero inteiro pela rede
    * @return O n�mero recebido
    */
   int recebeInt(void);

   /**
    * Envia um numero decimal (ponto flutuante) pela rede
    * @param f O n�mero a ser enviado
    * @return 0, se a opera��o foi mal sucedida, ou 1, em caso contr�rio 
    */
   short enviaFloat(float f){
      //short resp;
      
      limparBuffer();
      printf("AT+CMGF=1");
      putc(0x0d);//CR
      putc(0x0a);//Nova linha
      //sa�da esperada: OK
      
      carregarBuffer();
      
      delay_ms(500);
      
      if(bufferSerial[0]=='O'&& bufferSerial[1]=='K'){ 
         limparBuffer();
         printf("AT+CMGS=\"%s\"",destino);
         putc(0x0d);//CR
         putc(0x0a);//Nova linha
         printf("%f",f);
         putc(0x1a); //equivalente ao ctrl+z
         carregarBuffer();
         if(bufferSerial[0]=='O'&& bufferSerial[1]=='K'){
            return 1;
         }else{
            return 0;
         }
      }else{
         return 0;
      }     
   }

   /**
    * Recebe um n�mero decimal (ponto flutuante) pela rede
    * @return O n�mero recebido.
    */
   float recebeFloat(void);  
   
#endif /*_GPRS_H*/
