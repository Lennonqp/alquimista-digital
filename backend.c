#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_LINHA 4096
#define MAX_PERFUMES 600  
#define MAX_RESP 20000    

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

// --- FUNÇÕES AUXILIARES ---

void trim(char *str) {
    if (!str) return;
    char *ptr = str;
    int len = strlen(ptr);
    while (len > 0 && isspace(ptr[len - 1])) ptr[--len] = 0;
    while (*ptr && isspace(*ptr)) ptr++, len--;
    memmove(str, ptr, len + 1);
}

void limpar_string(char *str) {
    if (!str) return;
    str[strcspn(str, "\n")] = 0;
    str[strcspn(str, "\r")] = 0;
    if (str[0] == '"') memmove(str, str + 1, strlen(str)); 
    int len = strlen(str);
    if (len > 0 && str[len-1] == '"') str[len-1] = '\0'; 
    trim(str);
}

// COMPARAÇÃO INTELIGENTE (Ignora maiúsculas e parciais)
int contem_ignora_case(const char* texto_base, const char* busca) {
    if (!texto_base || !busca) return 0;
    char base_temp[200];
    char busca_temp[200];
    strncpy(base_temp, texto_base, 199); base_temp[199] = '\0';
    strncpy(busca_temp, busca, 199); busca_temp[199] = '\0';
    for(int i = 0; base_temp[i]; i++) base_temp[i] = tolower(base_temp[i]);
    for(int i = 0; busca_temp[i]; i++) busca_temp[i] = tolower(busca_temp[i]);
    if (strstr(base_temp, busca_temp) != NULL) return 1;
    return 0;
}

// --- CARREGAMENTO ---
void carregar_csv(char* nome_arquivo, char* genero_fixo) {
    FILE *arquivo = fopen(nome_arquivo, "r");
    if (!arquivo) {
        printf("[ERRO] Nao abriu %s\n", nome_arquivo);
        return;
    }

    char linha[MAX_LINHA];
    fgets(linha, sizeof(linha), arquivo); 

    while (fgets(linha, sizeof(linha), arquivo) && total_perfumes < MAX_PERFUMES) {
        linha[strcspn(linha, "\n")] = 0;

        char *t = strtok(linha, ","); 
        if(t) { limpar_string(t); strcpy(catalogo[total_perfumes].nome, t); }

        strtok(NULL, ","); 
        strtok(NULL, ","); 
        strcpy(catalogo[total_perfumes].genero, genero_fixo); 
        
        strtok(NULL, ","); 
        
        t = strtok(NULL, ","); if(t) { limpar_string(t); strcpy(catalogo[total_perfumes].periodo, t); }
        t = strtok(NULL, ","); if(t) { limpar_string(t); strcpy(catalogo[total_perfumes].ocasiao, t); }
        t = strtok(NULL, ","); if(t) { limpar_string(t); strcpy(catalogo[total_perfumes].estilo, t); }
        t = strtok(NULL, ","); if(t) { limpar_string(t); strcpy(catalogo[total_perfumes].estacao, t); }
        t = strtok(NULL, ","); if(t) { limpar_string(t); strcpy(catalogo[total_perfumes].orcamento, t); }
        t = strtok(NULL, ","); if(t) { limpar_string(t); strcpy(catalogo[total_perfumes].intensidade, t); }
        t = strtok(NULL, ","); if(t) { limpar_string(t); strcpy(catalogo[total_perfumes].preco, t); }
        t = strtok(NULL, ","); if(t) { limpar_string(t); strcpy(catalogo[total_perfumes].link, t); }

        catalogo[total_perfumes].ativo = 1; 
        total_perfumes++;
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
            if (strcmp(catalogo[i].genero, valor) == 0) manter = 1;
        }
        else if (strcmp(tipo, "ESTILO") == 0) {
            if (contem_ignora_case(catalogo[i].estilo, valor)) manter = 1;
        }
        else if (strcmp(tipo, "OCASIAO") == 0) {
            if (contem_ignora_case(catalogo[i].ocasiao, valor)) manter = 1;
        }
        else if (strcmp(tipo, "ESTACAO") == 0) {
            if (contem_ignora_case(catalogo[i].estacao, valor)) manter = 1;
        }
        else if (strcmp(tipo, "ORCAMENTO") == 0) {
            if (contem_ignora_case(catalogo[i].orcamento, valor)) manter = 1;
        }
        else if (strcmp(tipo, "PERIODO") == 0) {
            if (contem_ignora_case(catalogo[i].periodo, valor)) manter = 1;
        }
        
        catalogo[i].ativo = manter;
    }
}

// --- ARVORE ---
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
    
    carregar_csv("perfumes_masculinos.csv", "M"); 
    carregar_csv("perfumes_femininos.csv", "F");

    No* fim = criar_no("FIM", "NENHUM", "", NULL, NULL);

    // --- CORREÇÃO DAS PALAVRAS CHAVE ---
    // Usamos "Verão" (pega "Primavera / Verão" e "Verão / primavera")
    // Usamos "Todas" (pega "Todas as estações" e "Todas estações")
    
    No* p_estacao_2 = criar_no("Prefere algo para um clima mais quente ?", "ESTACAO", "Verão", fim, fim);
    No* p_estacao_1 = criar_no("Prefere algo versatil para todas as estacoes ?", "ESTACAO", "Todas", fim, p_estacao_2);

    No* p_periodo_2 = criar_no("Gostaria de um perfume para usar de Dia ?", "PERIODO", "Dia", p_estacao_1, p_estacao_1);
    No* p_periodo = criar_no("Gostaria de um perfume para qualquer periodo (Versatil) ?", "PERIODO", "Versatil", p_estacao_1, p_periodo_2);

    // Removemos "menor que" da busca, buscamos só "200" se possível, 
    // mas "menor que 200" costuma funcionar se o CSV estiver limpo.
    No* p_orcamento = criar_no("Seu orcamento é maior que R$ 200,00 ?", "ORCAMENTO", "maior", p_periodo, p_periodo);

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

    raiz = criar_no("O perfume é para o Genero Feminino ?", "GENERO", "F", f_estilo_1, m_estilo_1);
}

void resetar_catalogo() {
    for(int i=0; i < total_perfumes; i++) {
        catalogo[i].ativo = 1;
    }
}

void interagir_arvore(char* caminho_usuario, char* resposta) {
    if (raiz == NULL) inicializar_arvore();

    if (total_perfumes == 0) {
        sprintf(resposta, "RESULTADO:ERRO - Nenhum perfume carregado. Verifique o caminho dos CSVs.");
        return;
    }

    if (strlen(caminho_usuario) == 0) {
        resetar_catalogo();
        sprintf(resposta, "PERGUNTA:%s", raiz->texto);
        return;
    }

    No* atual = raiz;
    int len = strlen(caminho_usuario);
    
    resetar_catalogo(); 

    for (int i = 0; i < len; i++) {
        if (strcmp(atual->texto, "FIM") == 0) break;

        if (caminho_usuario[i] == '1') {
            aplicar_filtro(atual->tipo_filtro, atual->valor_filtro);
            if (atual->sim) atual = atual->sim;
        } else {
            if (strcmp(atual->tipo_filtro, "GENERO") == 0) aplicar_filtro("GENERO", "M");
            else if (strcmp(atual->tipo_filtro, "ORCAMENTO") == 0) aplicar_filtro("ORCAMENTO", "menor"); // Busca "menor"
            
            if (atual->nao) atual = atual->nao;
        }
    }

    if (strcmp(atual->texto, "FIM") == 0 || (atual->sim == NULL && atual->nao == NULL)) {
        char buffer_lista[MAX_RESP] = "";
        int cont = 0;

        for(int i=0; i < total_perfumes; i++) {
            if (catalogo[i].ativo == 1) {
                char item[300];
                sprintf(item, "%s - R$ %s|| %s \n", catalogo[i].nome, catalogo[i].preco, catalogo[i].link);
                
                if (strlen(buffer_lista) + strlen(item) < MAX_RESP - 100) {
                    strcat(buffer_lista, item);
                    cont++;
                }
            }
        }

        if (cont == 0) sprintf(resposta, "RESULTADO:Nenhum perfume encontrado.");
        else {
            char *final = (char*)malloc(MAX_RESP + 1000); 
            if (final) {
                sprintf(final, "RESULTADO:Encontramos %d opcoes:\n%s", cont, buffer_lista);
                strcpy(resposta, final);
                free(final);
            }
        }

    } else {
        sprintf(resposta, "PERGUNTA:%s", atual->texto);
    }
}
