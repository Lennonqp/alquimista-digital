#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_LINHA 4096
#define MAX_PERFUMES 300 
#define MAX_RESP 4096    

// --- ESTRUTURA DO PERFUME ---
typedef struct {
    char nome[100];
    char genero[5];      
    char periodo[50];    
    char ocasiao[50];    
    char estilo[50];     
    char estacao[50];
    char orcamento[50];
    char intensidade[50];
    char preco[30];
    char link[250];
    int ativo;           
} Perfume;

Perfume catalogo[MAX_PERFUMES];
int total_perfumes = 0;

// --- ESTRUTURA DA ÁRVORE ---
typedef struct No {
    char texto[200];       
    char tipo_filtro[20];  
    char valor_filtro[50]; 
    struct No *sim;
    struct No *nao;
} No;

No *raiz = NULL;

// --- FUNÇÃO LIMPEZA ---
void limpar_string(char *str) {
    // 1) Se a string for NULL, não há nada a limpar.
    if (!str) return;
     // 2) Remover quebra de linha: substitui '\n' e '\r' por '\0'.
    //    strcspn encontra o índice do primeiro desses caracteres.
    str[strcspn(str, "\n")] = 0;
    str[strcspn(str, "\r")] = 0;
    
      // 3) Se a string começar com aspas, deslocamos o texto para a esquerda.
    //    Isso remove a aspas inicial.
    if (str[0] == '"') {
        memmove(str, str + 1, strlen(str)); 
    }
     // 4) Verificar e remover aspas no final.
    int len = strlen(str);
    if (len > 0 && str[len-1] == '"') {
        str[len-1] = '\0'; 
    }
}


void carregar_csv(char* nome_arquivo) {
    FILE *arquivo = fopen(nome_arquivo, "r"); // Abre o arquivo em modo leitura ("r")
    if (!arquivo) {  // Caso o arquivo não abra
        printf("Erro ao abrir arquivo: %s\n", nome_arquivo); // Exibe mensagem de erro
        return; // Encerra a função caso não consiga abrir o arquivo
    }

    char linha[MAX_LINHA];
    // Pula cabeçalho
    fgets(linha, sizeof(linha), arquivo);

    while (fgets(linha, sizeof(linha), arquivo) && total_perfumes < MAX_PERFUMES) { // o fgets pega uma linha do arquivo para o buffer 'linha' e verifica se ainda há espaço no catálogo.
    //'total_perfumes' precisa ser menor do que 'MAX_PERFUMES', evitando ultrapassar o limite do vetor
        
    // Limpa o \n final da linha bruta
        linha[strcspn(linha, "\n")] = 0;

        // 1. COLUNA: NOME
        char *t = strtok(linha, ","); //declara um ponteiro, e strtok é uma função que divide uma string em substrings usando caracteres separadores
        if(t) { limpar_string(t); strcpy(catalogo[total_perfumes].nome, t); } //limpar: Remove espaços extras //strcpy: Copia o conteúdo da string 't' para o campo 'nome' do perfume no catálogo

        // 2. COLUNA: LINHA (Ignorar, mas precisamos ler para passar o cursor)
        t = strtok(NULL, ","); 
        
        // 3. COLUNA: GENERO
        t = strtok(NULL, ",");
        if(t) { 
            limpar_string(t); 
            strcpy(catalogo[total_perfumes].genero, t);
        }
        
       
        
        strtok(NULL, ","); // Pula Familia Olfativa 
        
        
        // A partir daqui, ambos os arquivos estão alinhados na coluna PERIODO
        
        // 4. PERIODO
        t = strtok(NULL, ","); if(t) { limpar_string(t); strcpy(catalogo[total_perfumes].periodo, t); }
        // 5. OCASIAO
        t = strtok(NULL, ","); if(t) { limpar_string(t); strcpy(catalogo[total_perfumes].ocasiao, t); }
        // 6. ESTILO
        t = strtok(NULL, ","); if(t) { limpar_string(t); strcpy(catalogo[total_perfumes].estilo, t); }
        // 7. ESTACAO
        t = strtok(NULL, ","); if(t) { limpar_string(t); strcpy(catalogo[total_perfumes].estacao, t); }
        // 8. ORCAMENTO
        t = strtok(NULL, ","); if(t) { limpar_string(t); strcpy(catalogo[total_perfumes].orcamento, t); }
        // 9. INTENSIDADE
        t = strtok(NULL, ","); if(t) { limpar_string(t); strcpy(catalogo[total_perfumes].intensidade, t); }
        // PRECO
        t = strtok(NULL, ","); if(t) { limpar_string(t); strcpy(catalogo[total_perfumes].preco, t); }
        // LINK
        t = strtok(NULL, ","); if(t) { limpar_string(t); strcpy(catalogo[total_perfumes].link, t); }

        catalogo[total_perfumes].ativo = 1; // Marca o perfume atual como ativo no catálogo (1 = ativo)
        total_perfumes++; // Avança o índice para o próximo perfume a ser inserido
    }
    fclose(arquivo);
}

// --- FILTRAGEM ---
void aplicar_filtro(char* tipo, char* valor) {
    if (strcmp(tipo, "NENHUM") == 0) return; 

    for (int i = 0; i < total_perfumes; i++) {
        if (catalogo[i].ativo == 0) continue;

        int manter = 0; 

        if (strcmp(tipo, "GENERO") == 0) {
            // Comparação exata
            if (strcmp(catalogo[i].genero, valor) == 0) manter = 1;
        }
        else if (strcmp(tipo, "ESTILO") == 0) {
            if (strstr(catalogo[i].estilo, valor) != NULL) manter = 1;
        }
        else if (strcmp(tipo, "OCASIAO") == 0) {
            if (strstr(catalogo[i].ocasiao, valor) != NULL) manter = 1;
        }
        else if (strcmp(tipo, "ESTACAO") == 0) {
            if (strstr(catalogo[i].estacao, valor) != NULL) manter = 1;
        }
        else if (strcmp(tipo, "ORCAMENTO") == 0) {
            if (strstr(catalogo[i].orcamento, valor) != NULL) manter = 1;
        }
        else if (strcmp(tipo, "PERIODO") == 0) {
            if (strstr(catalogo[i].periodo, valor) != NULL) manter = 1;
        }
        
        catalogo[i].ativo = manter;
    }
}

// --- FUNÇÕES DA ÁRVORE ---
No* criar_no(char* texto, char* tipo, char* valor, No* sim, No* nao) {
    No* novo = (No*)malloc(sizeof(No));
    strcpy(novo->texto, texto);
    strcpy(novo->tipo_filtro, tipo);
    strcpy(novo->valor_filtro, valor);
    novo->sim = sim;
    novo->nao = nao;
    return novo;
}

void inicializar_arvore() {
    total_perfumes = 0;
    
    carregar_csv("perfumes_masculinos.csv"); 
    carregar_csv("perfumes_femininos.csv");

    No* fim = criar_no("FIM", "NENHUM", "", NULL, NULL);

    // --- RAMOS DE PERGUNTAS ---

    // Estação
    No* p_estacao_2 = criar_no("Prefere algo para um clima mais quente ?", "ESTACAO", "Primavera / Verão", fim, fim);
    No* p_estacao_1 = criar_no("Prefere algo versatil para todas as estacoes ?", "ESTACAO", "Todas as estações", fim, p_estacao_2);

    // Periodo
    No* p_periodo_2 = criar_no("Gostaria de um perfume para usar de Dia ?", "PERIODO", "Dia", p_estacao_1, p_estacao_1);
    No* p_periodo = criar_no("Gostaria de um perfume para qualquer periodo (Versatil) ?", "PERIODO", "Versatil", p_estacao_1, p_periodo_2);

    // Orçamento
    No* p_orcamento = criar_no("Seu orcamento é maior que R$ 200,00 ?", "ORCAMENTO", "maior que 200", p_periodo, p_periodo);

    // --- RAMO MASCULINO ---
    No* m_estilo_14 = criar_no("Gostaria de algo mais Atraente ?", "ESTILO", "Misterioso", p_orcamento, p_orcamento);
    No* m_estilo_13 = criar_no("Gostaria de algo Marcante ?", "ESTILO", "Prático", p_orcamento, m_estilo_14);
    No* m_estilo_12 = criar_no("Gostaria de algo mais Misterioso ?", "ESTILO", "Misterioso", p_orcamento, m_estilo_13);
    No* m_estilo_11 = criar_no("Gostaria de algo mais Prático ?", "ESTILO", "Prático", p_orcamento, m_estilo_12);
    No* m_estilo_10 = criar_no("Gostaria de algo no estilo mais Aventureiro ?", "ESTILO", "Aventureiro", p_orcamento, m_estilo_11);
    No* m_estilo_9 = criar_no("Gostaria de algo que transmita Confiança ?", "ESTILO", "Confiante", p_orcamento, m_estilo_10);
    No* m_estilo_8 = criar_no("Gostaria de algo mais Versátil ?", "ESTILO", "Versátil", p_orcamento, m_estilo_9);
    No* m_estilo_7 = criar_no("Gostaria de algo mais Prático ?", "ESTILO", "Prático", p_orcamento, m_estilo_8);
    No* m_estilo_6 = criar_no("Gostaria de algo mais Elegante ?", "ESTILO", "Elegante", p_orcamento, m_estilo_7);
    No* m_estilo_5 = criar_no("Gostaria de um estilo mais Culto ?", "ESTILO", "Culto", p_orcamento, m_estilo_6);
    No* m_estilo_4 = criar_no("Gostaria de um estilo Determinado/Energético ?", "ESTILO", "Determinado", p_orcamento, m_estilo_5);
    No* m_estilo_3 = criar_no("Gostaria de um estilo Jovem/Divertido ?", "ESTILO", "Jovem", p_orcamento, m_estilo_4);
    No* m_estilo_2 = criar_no("Gostaria de um estilo Sedutor ?", "ESTILO", "Sedutor", p_orcamento, m_estilo_3);
    No* m_estilo_1 = criar_no("Gostaria de um estilo Sofisticado ?", "ESTILO", "Sofisticado", p_orcamento, m_estilo_2);
    
    // --- RAMO FEMININO ---
    No* f_estilo_9 = criar_no("Gostaria de um estilo mais Ousado ?", "ESTILO", "Ousada", p_orcamento, p_orcamento);
    No* f_estilo_8 = criar_no("Gostaria de algo mais Jovem ?", "ESTILO", "Jovem", p_orcamento, f_estilo_9);
    No* f_estilo_7 = criar_no("Gostaria de um estilo mais Sofisticado ?", "ESTILO", "Sofisticada", p_orcamento, f_estilo_8);
    No* f_estilo_6 = criar_no("Gostaria de um estilo mais Marcante ?", "ESTILO", "Marcante", p_orcamento, f_estilo_7);
    No* f_estilo_5 = criar_no("Gostaria de um estilo mais Versátil ?", "ESTILO", "Versatil", p_orcamento, f_estilo_6);
    No* f_estilo_4 = criar_no("Gostaria de um estilo Elegante ?", "ESTILO", "Elegante", p_orcamento, f_estilo_5);
    No* f_estilo_3 = criar_no("Gostaria de um estilo Jovem ?", "ESTILO", "Jovem", p_orcamento, f_estilo_4);
    No* f_estilo_2 = criar_no("Gostaria de um estilo Sensual ?", "ESTILO", "Sensual", p_orcamento, f_estilo_3);
    No* f_estilo_1 = criar_no("Gostaria de um estilo Delicado ?", "ESTILO", "Delicada", p_orcamento, f_estilo_2);

    // --- RAIZ ---
    raiz = criar_no("O perfume é para o Genero Feminino ?", "GENERO", "F", f_estilo_1, m_estilo_1);
}
// Transforma toda a lista de perfumes em 1 novamente
void resetar_catalogo() {
    for(int i=0; i < total_perfumes; i++) {
        catalogo[i].ativo = 1;
    }
}

//Função que faz a interação com o código em python
void interagir_arvore(char* caminho_usuario, char* resposta) {
    if (raiz == NULL) inicializar_arvore();

    if (strlen(caminho_usuario) == 0) {
        resetar_catalogo();
        sprintf(resposta, "PERGUNTA:%s", raiz->texto);
        return;
    }// verifica se o usuário não fez nenhuma ação ele faz a pergunta da raiz "é Feminino ?"

    No* atual = raiz;
    int len = strlen(caminho_usuario);
    
    resetar_catalogo();// Transforma todos os elementos da lista em ativos novamente =1 

    // Laço de repetição que vai parar quando o usuário chegar na resposta
    for (int i = 0; i < len; i++) {
        if (strcmp(atual->texto, "FIM") == 0) break; //Verifica se o usuário chegou no nó folha

        if (caminho_usuario[i] == '1') {
            aplicar_filtro(atual->tipo_filtro, atual->valor_filtro);
            if (atual->sim) atual = atual->sim;
        } // Verifica se o usuário escolheu SIM(1) e aplica o filtro e apaga o que não for da condição, depois move-se para a próxima sub-árvore "SIM"
        else {
            // Lógica do NÃO
            if (strcmp(atual->tipo_filtro, "GENERO") == 0) aplicar_filtro("GENERO", "M");
            else if (strcmp(atual->tipo_filtro, "ORCAMENTO") == 0) aplicar_filtro("ORCAMENTO", "menor que 200");
            
            if (atual->nao) atual = atual->nao;
        }
    }// Verifica se o filtro for GENERO OU ORCAMENTO, Ele remove da lista tudo que não for do genero masculino ou o orçamento maior que 200 (outros filtros ele somente vai passar para frente) e vai para a sub-árvore do não

    if (strcmp(atual->texto, "FIM") == 0 || (atual->sim == NULL && atual->nao == NULL)) {
        char buffer_lista[MAX_RESP] = "";
        int cont = 0;

        for(int i=0; i < total_perfumes; i++) {
            if (catalogo[i].ativo == 1) {
                char item[300];
                sprintf(item, "• %s - %s|| %s \n", catalogo[i].nome, catalogo[i].preco, catalogo[i].link);
                if (strlen(buffer_lista) + strlen(item) < MAX_RESP - 100) {
                    strcat(buffer_lista, item);
                    cont++;
                }
            }
        }// Verifica se o Nó atual for FIM ele mostra uma mensagem com as informações do item se tiver item na lista

        if (cont == 0) sprintf(resposta, "RESULTADO:Nenhum perfume encontrado.");// Se a variável cont =0 significa que não encontrou nenhum perfume com aqueles filtros combinados
        else {
            char final[MAX_RESP + 100];
            sprintf(final, "RESULTADO:Encontramos %d opcoes:\n%s", cont, buffer_lista);
            strcpy(resposta, final);
        }// se Cont diferente de 0 retorna a resposta final

    } else {
        sprintf(resposta, "PERGUNTA:%s", atual->texto);
    }// Se não for é pergunta
}