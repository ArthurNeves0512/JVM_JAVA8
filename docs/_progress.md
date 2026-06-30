# Progresso da Documentação

## Status geral

| Fase | Status | Descrição |
|------|--------|-----------|
| Fase 1 — Levantamento e mapeamento | ✅ Concluída | `_map.md` gerado com 7 módulos, 27 arquivos mapeados |
| Fase 2 — Extração de funções | ✅ Concluída | `_functions.json` gerado com 60+ funções documentadas |
| Fase 3 — Estruturas de dados | ✅ Concluída | `_structs.json` gerado com 14 structs documentadas |
| Fase 4 — Diagramas de comunicação | ✅ Concluída | 7 diagramas Mermaid em `_diagrams/*.mmd` |
| Fase 5 — CSS padrão | ✅ Concluída | `docs/html/style.css` com tema escuro, sidebar, cards |
| Fase 6 — Páginas HTML por módulo | ✅ Concluída | 16 páginas geradas em `docs/html/modules/` |
| Fase 7 — Página de estruturas | ✅ Concluída | `docs/html/structs.html` com 14 structs |
| Fase 8 — Diagrama geral e índice | ✅ Concluída | `docs/html/diagram.html` e `docs/html/index.html` |
| Fase 9 — Validação final | ✅ Concluída | 16/16 módulos, 0 links quebrados |

## Fase 1 — Concluída

**O que foi feito:**
- Listados todos os 27 arquivos `.c`/`.h` do projeto (excluindo `tests/` e `build/`)
- Identificados 7 módulos funcionais: `main`, `file`, `types`, `class_loader`, `interpreter`, `printer`, `utils`
- Gerado `docs/_map.md`

## Fase 2 — Concluída

**O que foi feito:**
- Lidos todos os 16 arquivos `.c` do projeto
- Extraídas 60+ funções com assinatura, linha, resumo, parâmetros, retorno e calls
- Gerado `docs/_functions.json`

## Fase 3 — Concluída

**O que foi feito:**
- Identificadas 14 structs/typedefs centrais da JVM
- Documentados todos os campos com tipo e descrição
- Gerado `docs/_structs.json`

## Fase 4 — Concluída

**Diagramas gerados:**
- `_diagrams/00_overview.mmd` — visão de alto nível entre módulos
- `_diagrams/01_main.mmd` — chamadas do main()
- `_diagrams/02_class_loader.mmd` — cadeia de carregamento de classes
- `_diagrams/03_interpreter.mmd` — pipeline de interpretação
- `_diagrams/04_method_invoke.mmd` — despacho de invocações
- `_diagrams/05_native_methods.mmd` — registro de métodos nativos
- `_diagrams/06_heap.mmd` — alocação na heap

## Fase 5 — Concluída

- `docs/html/style.css`: tema escuro, sidebar fixa, cards de função colapsáveis, highlight.js, mermaid.js

## Fase 6 — Concluída

- 16 páginas HTML geradas em `docs/html/modules/`:
  - main.html, read_file.html, dot_class.html, attribute.html, opcodes.html
  - loader.html, fields_interfaces.html, methods.html, interpreter.html, basic_ops.html
  - jvm_stack.html, heap.html, method_invoke.html, native_methods.html, printer.html, args.html

## Fase 7 — Concluída

- `docs/html/structs.html`: 14 structs com tabelas de campos e links para funções

## Fase 8 — Concluída

- `docs/html/diagram.html`: diagrama de alto nível Mermaid + grid de módulos com links
- `docs/html/index.html`: visão geral, estatísticas, pipeline, grid de módulos, busca de funções

## Fase 9 — Concluída

- 16/16 páginas de módulo presentes
- 0 links de arquivo quebrados
- 3 âncoras corrigidas (printExceptionsAttribute, printSourceFileAttribute, printInnerClassesAttribute → attribute.html sem âncora)
- Documentação completa e navegável via `file://`
