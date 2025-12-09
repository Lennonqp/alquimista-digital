from flask import Flask, render_template, session, redirect, url_for
import ctypes
import os
import sys

app = Flask(__name__)
app.secret_key = 'alquimista' # Necessário para guardar o histórico do usuário

# --- 1. CONFIGURAÇÃO DO BACKEND C ---
pasta_script = os.path.dirname(os.path.abspath(__file__))
os.chdir(pasta_script)

if os.name == 'nt':
    lib_name = "backend.dll"
else:
    lib_name = "./backend.so"

try:
    backend = ctypes.CDLL(os.path.join(pasta_script, lib_name))
    # Configurar tipos
    backend.interagir_arvore.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
    backend.interagir_arvore.restype = None
except OSError:
    print("ERRO: Não encontrei o backend_arvore.dll. Compile o código C primeiro!")
    sys.exit()

def consultar_c(caminho):
    buffer_resp = ctypes.create_string_buffer(4096)
    caminho_bytes = caminho.encode('utf-8')
    backend.interagir_arvore(caminho_bytes, buffer_resp)
    return buffer_resp.value.decode('utf-8', errors='ignore')

# --- 2. ROTAS DO SITE ---

@app.route('/')
def index():
    if 'caminho' not in session:
        session['caminho'] = ""

    # Chama o C
    resposta_raw = consultar_c(session['caminho'])
    
    dados = {}
    
    if resposta_raw.startswith("PERGUNTA:"):
        dados['tipo'] = 'pergunta'
        dados['texto'] = resposta_raw.replace("PERGUNTA:", "")
        
    elif resposta_raw.startswith("RESULTADO:"):
        dados['tipo'] = 'resultado'
        texto_bruto = resposta_raw.replace("RESULTADO:", "")
        
        # Vamos transformar o texto numa lista de objetos inteligentes
        lista_processada = []
        
        # Divide por linhas primeiro
        linhas = texto_bruto.split("\n")
        
        for linha in linhas:
            if linha.strip() != "":
                item = {}
                # Verifica se tem o separador do link '||'
                if "||" in linha:
                    partes = linha.split("||")
                    item['nome'] = partes[0]  # Parte antes do || (Nome + Preço)
                    item['link'] = partes[1]  # Parte depois (https://...)
                else:
                    item['nome'] = linha
                    item['link'] = None       # Não tem link
                
                lista_processada.append(item)
                
        dados['lista'] = lista_processada # Passamos a lista pronta
        
    elif resposta_raw.startswith("FIM:"):
        dados['tipo'] = 'fim'
        
    return render_template('index.html', dados=dados)

@app.route('/responder/<escolha>')
def responder(escolha):
    # Escolha é '1' (Sim) ou '0' (Não)
    if 'caminho' in session:
        session['caminho'] += escolha
    return redirect(url_for('index'))

@app.route('/reiniciar')
def reiniciar():
    session['caminho'] = ""
    return redirect(url_for('index'))

if __name__ == '__main__':
    # Debug=True faz o site recarregar se mudares o código
    app.run(debug=True)