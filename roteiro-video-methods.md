# Roteiro — Vídeo de 3 minutos | Implementação de Methods

Vídeo curto e objetivo, foco no código implementado. Tempo-alvo: ~3 min.
Fale de forma natural; os tempos abaixo são só guia.

**Slides usados:**
1. Methods | implementação (código de `readMethodsCount` e `readMethods`)
2. Methods (deep-dive nos atributos)
3. Testes | methods_count e methods[]

---

## Abertura — sem slide ou já no slide 1 (0:00 – 0:18)

> "Nesta parte do projeto eu implementei a leitura do **array de métodos** do arquivo `.class`.
> Em poucas palavras: fazer a JVM ler e guardar **quais métodos** a classe tem — e cobrir isso com testes."

---

## Slide 1 — "Methods | implementação" (0:18 – 1:10)

*(mostre o slide 1 — código de readMethodsCount e readMethods)*

> "São duas funções no arquivo `methods.h`.
>
> A primeira, `readMethodsCount`, é simples: lê um inteiro de 2 bytes do arquivo e guarda em `cf->methods_count`. Apenas isso.
>
> A segunda, `readMethods`, faz o trabalho de verdade. Primeiro ela aloca o array `methods[]` dentro da struct `ClassFile`, com tamanho `methods_count`. Depois entra num laço, e para cada método lê quatro campos: `access_flags`, `name_index`, `descriptor_index` e `attributes_count` — armazenando cada um direto na posição `cf->methods[i]`."

*(aponte para o laço interno do código)*

> "E tem esse laço aninhado aqui no final, que cuida dos atributos de cada método. É justamente sobre ele que falo no próximo slide."

---

## Slide 2 — "Methods" (1:10 – 2:05)

*(mude para o slide 2 — o "porquê" do consumo de atributos)*

> "Cada método pode ter atributos — por exemplo o atributo `Code`, que guarda o bytecode.
>
> Nesta etapa eu ainda **não exibo** esses atributos. Mas, mesmo assim, eu **preciso lê-los e descartá-los** byte a byte.
>
> O motivo é que o leitor é **sequencial**: ele percorre o arquivo do início ao fim. Se eu não consumir esses bytes, o leitor **perde a posição** no arquivo — e tudo que vem depois é lido errado.
>
> Por isso esse laço interno lê o tamanho do atributo (`len`) e consome exatamente aquele número de bytes."

---

## Slide 3 — "Testes" (2:05 – 2:50)

*(mude para o slide 3 — testes)*

> "Para garantir que a leitura está correta, escrevi **7 testes unitários**, no arquivo `test_methods.c`.
>
> A maioria usa **bytes sintéticos**: eu monto na mão uma sequência de bytes representando métodos e verifico se os campos foram lidos com os valores certos — `methods_count`, `name_index`, `descriptor_index`. Tem inclusive um teste que confere especificamente se os bytes dos atributos são **pulados** na posição certa.
>
> E tem o **teste de integração**, o `test_opa_methods_count`, que está aí à direita: ele abre um `.class` de verdade, o `Opa.class`, roda toda a cadeia de leitura — setup, interfaces, fields e então os métodos — e verifica que a classe tem exatamente **2 métodos**.
>
> Resultado: os **7 testes passam**."

---

## Fechamento (2:50 – 3:00)

> "Resumindo: a leitura dos métodos está implementada, armazenada na struct `ClassFile` e coberta por testes automatizados. É isso, obrigado."

---

### Dicas rápidas de gravação
- Ritmo tranquilo: o roteiro tem folga dentro dos 3 minutos.
- Ao falar de código, **aponte** para o trecho na tela em vez de ler linha por linha.
- Se errar, pause 2 segundos e repita a frase — facilita o corte na edição.
