#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

// --- AUMENTO DE MEMÓRIA PARA EVITAR CORTES NA LISTA ---
#define MAX_LINHA 4096
#define MAX_PERFUMES 500  
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

// --- FUNÇÕES DE LIMPEZA (CRITICAS) ---

// Remove espaços do começo e do fim (ex: " Floral " vira "Floral")
void trim(char *str) {
    if (!str) return;
    char *ptr = str;
    int len = strlen(ptr);

    // Remove do final
    while (len > 0 && isspace(ptr[len - 1])) ptr[--len] = 0;

    // Remove do começo
    while (*ptr && isspace(*ptr)) ptr++, len--;

    // Move o texto limpo para o início
    memmove(str, ptr, len + 1);
}

void limpar_string(char *str) {
    if (!str) return;
    str[strcspn(str, "\n")] = 0;
    str[strcspn(str, "\r")] = 0;
    
    // Remove aspas
    if (str[0] == '"') {
        memmove(str, str + 1, strlen(str)); 
    }
    int len = strlen(str);
    if (len > 0 && str[len-1] == '"') {
        str[len-1] = '\0'; 
    }
    
    // Aplica o trim final
    trim(str);
}

// --- CARREGAMENTO ROBUSTO ---
void carregar_csv(char* nome_arquivo, char* genero_fixo) {
    FILE *arquivo = fopen(nome_arquivo, "r");
    if (!arquivo) {
        printf("[ERRO] Nao foi possivel abrir o arquivo: %s\n", nome_arquivo);
        return;
    }

    char linha[MAX_LINHA];
    // Pula cabeçalho
    fgets(linha, sizeof(linha), arquivo);

    while (fgets(linha, sizeof(linha), arquivo) && total_perfumes < MAX_PERFUMES) {
        linha[strcspn(linha, "\n")] = 0;

        // 1. NOME
        char *t = strtok(linha, ","); 
        if(t) { limpar_string(t); strcpy(catalogo[total_perfumes].nome, t); }

        // 2. LINHA (Ignorar)
        strtok(NULL, ","); 
        
        // 3. GENERO (Força o fixo)
        strtok(NULL, ","); 
        strcpy(catalogo[total_perfumes].genero, genero_fixo);
        
        // 4. FAMILIA (Ignorar)
        strtok(NULL, ","); 
        
        // 5. PERIODO
        t = strtok(NULL, ","); if(t) { limpar_string(t); strcpy(catalogo[total_perfumes].periodo, t); }
        // 6. OCASIAO
        t = strtok(NULL, ","); if(t) { limpar_string(t); strcpy(catalogo[total_perfumes].ocasiao, t); }
        // 7. ESTILO
        t = strtok(NULL, ","); if(t) { limpar_string(t); strcpy(catalogo[total_perfumes].estilo, t); }
        // 8. ESTACAO
        t = strtok(NULL, ","); if(t) { limpar_string(t); strcpy(catalogo[total_perfumes].estacao, t); }
        // 9. ORCAMENTO
        t = strtok(NULL, ","); if(t) { limpar_string(t); strcpy(catalogo[total_perfumes].orcamento, t); }
        // 10. INTENSIDADE
        t = strtok(NULL, ","); if(t) { limpar_string(t); strcpy(catalogo[total_perfumes].intensidade, t); }
        // 11. PRECO
        t = strtok(NULL, ","); if(t) { limpar_string(t); strcpy(catalogo[total_perfumes].preco, t); }
        // 12. LINK
        t = strtok(NULL, ","); if(t) { limpar_string(t); strcpy(catalogo[total_perfumes].link, t); }

        // --- DIAGNÓSTICO DE DESALINHAMENTO ---
        // Se o preço não tiver ponto nem for vazio, ou link não tiver http, avisa no terminal
        if (strlen(catalogo[total_perfumes].preco) > 0 && strchr(catalogo[total_perfumes].preco, '.') == NULL && strstr(catalogo[total_perfumes].preco, "R$") == NULL) {
             // Aceita R$ ou ponto decimal. Se não tiver nenhum
