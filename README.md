# Buscador de Certificados UTFPR

Ferramenta automatizada para varrer as listas públicas do Sistema de Gestão de Certificados Eletrônicos da UTFPR. O projeto é dividido em duas frentes independentes: um aplicativo nativo para Windows (Desktop) e uma aplicação baseada em navegador (Web).

## 📁 Estrutura do Repositório

* **`/desktop`**: Código-fonte da versão aplicativo desktop (C++/Qt).
* **`/web`**: Código-fonte da versão baseada em navegador (Node.js).

---

## 🖥️ Versão Desktop (Windows)

Aplicativo nativo com interface gráfica própria, processamento paralelo de requisições de rede e terminal de logs integrado. Não requer instalação de dependências ou interpretadores por parte do usuário final.

### Tecnologias Utilizadas

* **C++17**
* **Qt 6** (Componentes: Widgets, Network, Core)
* **CMake**

### Como Usar (Usuários Finais)

1. Acesse a página de **Releases** no repositório.
2. Baixe o arquivo `.zip` da última versão estável (ex: `v1.0.0`).
3. Extraia o conteúdo para uma pasta no seu computador.
4. Execute o arquivo `.exe` localizado na raiz da pasta extraída. Mantenha as bibliotecas `.dll` na mesma pasta do executável.

### Compilação (Desenvolvedores)

1. Abra o arquivo `CMakeLists.txt` no Qt Creator (Kit MinGW 64-bit).
2. Configure o Build para modo **Release** e compile o projeto.
3. Copie o arquivo `.exe` gerado na pasta de compilação para um novo diretório vazio.
4. Abra o terminal *Qt Command Prompt*, navegue até o novo diretório e execute o utilitário de implantação:

   ```bash
   windeployqt nome_do_executavel.exe
   ```

---

## 🌐 Versão Web (Node.js)

Interface web acessada via navegador. Utiliza automação para mapeamento dos eventos disponíveis e WebSockets para prover feedback em tempo real no front-end.

### Tecnologias Utilizadas

* **Node.js** (Back-end)
* **Express** (Servidor Web)
* **Socket.io** (Comunicação em tempo real para o terminal virtual)
* **Puppeteer** (Automação de navegador headless para extração de IDs)
* **Cheerio** (Extração e varredura rápida de HTML)

### Pré-requisitos

* Node.js instalado (Recomendada versão LTS 20 ou superior).

### Instalação

1. Pelo terminal, navegue até o diretório da versão web:

   ```bash
   cd web
   ```

2. Instale as dependências do projeto:

   ```bash
   npm install express socket.io puppeteer cheerio
   ```

### Como Executar

1. Com as bibliotecas instaladas e dentro da pasta `/web`, inicie o servidor:

   ```bash
   node server.js
   ```

2. Abra o navegador e acesse: `http://localhost:3000`

3. Insira o nome (ou parte dele) e o ano desejado. Clique em "Buscar Agora". O terminal exibirá o progresso e os links dos certificados encontrados.
