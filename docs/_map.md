# Mapa do Projeto — JVM implementada em C

> Gerado na Fase 1. Exclui arquivos de teste (`tests/`) e build artifacts.

## Módulos e arquivos

### Módulo: `main`
| Arquivo | Papel | Descrição |
|---|---|---|
| `src/main.c` | Ponto de entrada | Orquestra toda a pipeline: parseia argumentos, lê o `.class`, carrega o `ClassFile`, imprime informações e executa a JVM. |

---

### Módulo: `file` — Leitura de arquivo binário
| Arquivo | Papel | Descrição |
|---|---|---|
| `src/lib/file/read_file.c` | Leitor de arquivo | Abre o arquivo `.class` e retorna um ponteiro `FILE*` para leitura sequencial. |
| `src/lib/file/read_file.h` | Interface pública | Declara `readFile()`. |
| `src/lib/file/read_byte.h` | Leitura de bytes big-endian | Define funções inline `u1Read`, `u2Read`, `u4Read` para ler 1, 2 e 4 bytes no formato big-endian exigido pelo formato `.class`. |

---

### Módulo: `types` — Tipos e estruturas de dados do formato `.class`
| Arquivo | Papel | Descrição |
|---|---|---|
| `src/lib/types/dataTypes.h` | Tipos primitivos | Define os aliases `u1`, `u2`, `u4`, `u8` mapeando para `uint8_t`…`uint64_t`. |
| `src/lib/types/consts.h` | Enumerações de constantes JVM | Define `CONSTANT_POOL_TAGS` (tags do pool de constantes) e `ACCESS_MODIFIERS` (flags de acesso). |
| `src/lib/types/constant_pool.h` | Estrutura do pool de constantes | Define `cp_info`, uma tagged union que representa qualquer entrada do constant pool (classe, método, campo, string, int, float, etc.). |
| `src/lib/types/class_file/cp_info.h` | Estruturas de entradas do CP | Declara todas as structs concretas do constant pool: `CONSTANT_Class_info`, `CONSTANT_Methodref_info`, `CONSTANT_Utf8_info`, etc. |
| `src/lib/types/class_file/attributes_info.h` | Estrutura genérica de atributo | Define `attribute_info` (cabeçalho genérico de atributo: índice de nome + bytes raw). |
| `src/lib/types/class_file/methods_info.h` | Estruturas de métodos e campos | Define `field_info` e `method_info` conforme a especificação do formato `.class`. |
| `src/lib/types/class_file/dot_class.h` | Estrutura raiz ClassFile | Define a struct `ClassFile` que agrega todos os componentes de um arquivo `.class`: versão, CP, flags, interfaces, campos, métodos, atributos e ponteiro para a superclasse. |
| `src/lib/types/class_file/dot_class.c` | Alocação e liberação de ClassFile | Implementa `allocClassFile()` e `freeClassFile()`, gerenciando a memória da estrutura raiz. |
| `src/lib/types/attribute.h` | Estruturas de atributos decodificados | Define structs para atributos concretos: `Code_attribute`, `ConstantValue_attribute`, `Exceptions_attribute`, `LineNumberTable_attribute`, etc. Declara funções de leitura, impressão e liberação. |
| `src/lib/types/attribute.c` | Parser e printer de atributos | Implementa a leitura (decodificação) e impressão de todos os atributos JVM suportados, além de `getUtf8()` para resolver strings do CP. |
| `src/lib/types/opcodes.h` | Enumeração de opcodes | Define o enum `Opcode` com todos os bytecodes JVM (OP_NOP, OP_IADD, OP_INVOKEVIRTUAL, etc.). |
| `src/lib/types/opcodes.c` | Mapeamento opcode → nome | Provavelmente implementa função para converter código numérico de opcode em string legível (para debug/print). |

---

### Módulo: `class_loader` — Carregador de classes
| Arquivo | Papel | Descrição |
|---|---|---|
| `src/lib/class_loader/loader.c` | Carregador principal de `.class` | Lê e decodifica o arquivo `.class` sequencialmente: magic, versão, constant pool, flags, this/super class, e atributos do ClassFile. Também carrega classes do sistema de arquivos em tempo de execução. |
| `src/lib/class_loader/loader.h` | Interface do carregador | Declara todas as funções de leitura do ClassFile e utilitários como `getClassName`, `loadClassFile`, `setClassDir`. |
| `src/lib/class_loader/fields_interfaces.c` | Leitor de campos e interfaces | Lê e decodifica as seções de interfaces e campos do arquivo `.class`. |
| `src/lib/class_loader/fields_interfaces.h` | Interface para campos/interfaces | Declara `readInterfaces()` e `readFields()`. |
| `src/lib/class_loader/methods.c` | Leitor de métodos | Lê e decodifica a seção de métodos do arquivo `.class`, incluindo seus atributos (como `Code`). |
| `src/lib/class_loader/methods.h` | Interface para métodos | Declara `readMethodsCount()`, `readMethods()` e `freeMethods()`. |

---

### Módulo: `interpreter` — Interpretador de bytecode
| Arquivo | Papel | Descrição |
|---|---|---|
| `src/lib/interpreter/interpreter.c` | Ponto de entrada do interpretador | Implementa `executaJVM()`, que inicializa a JVM (registros nativos, stack), localiza o método `main` e inicia a execução. |
| `src/lib/interpreter/interpreter.h` | Interface do interpretador | Declara `executaJVM(ClassFile *cf)`. |
| `src/lib/interpreter/basic_ops.c` | Motor de execução de bytecodes | Implementa `executaFrame()`, o grande switch-case que despacha todos os opcodes JVM suportados: operações aritméticas, load/store, controle de fluxo, arrays, campos, invocações. |
| `src/lib/interpreter/basic_ops.h` | Estruturas de frame e slot | Define `Slot` (valor tipado da pilha de operandos), `Frame` (frame de ativação com pilha, variáveis locais, código e PC), `JVMArray` e constantes de tipo. |
| `src/lib/interpreter/jvm_stack.c` | Pilha de frames JVM | Implementa a pilha de chamadas da JVM (`JVMStack`): criação, push/pop de frames e acesso ao frame atual. |
| `src/lib/interpreter/jvm_stack.h` | Interface da pilha de frames | Declara `JVMStack` e as operações `criaJVMStack`, `empilhaFrame`, `desempilhaFrame`, `frameAtual`, `liberaJVMStack`. |
| `src/lib/interpreter/heap.c` | Heap de objetos | Implementa alocação de objetos (`alocaObjeto`), busca de campos por nome (`buscaCampo`) e liberação de toda a heap (`liberaHeap`). |
| `src/lib/interpreter/heap.h` | Interface da heap | Declara `HeapObject` (objeto em runtime com nome de classe, campos e dados nativos) e as funções de manipulação da heap. |
| `src/lib/interpreter/method_invoke.c` | Invocação de métodos | Implementa os handlers para `invokestatic`, `invokevirtual` e `invokespecial`: resolve o método pelo constant pool, monta o novo frame com argumentos e o executa. |
| `src/lib/interpreter/method_invoke.h` | Interface de invocação | Declara `execInvokestatic`, `execInvokevirtual`, `execInvokespecial` e utilitários como `conta_args`, `buscaMetodoClasse`, `encontraCodeAttr`. |
| `src/lib/interpreter/native_methods.c` | Métodos nativos | Implementa o registro e despacho de métodos nativos Java simulados (ex: `System.out.println`, `StringBuilder`, métodos de `Math`). |
| `src/lib/interpreter/native_methods.h` | Interface de métodos nativos | Declara o tipo `NativeFn`, `initNativeMethods`, `lookupNative` e `isNativeClass`. |

---

### Módulo: `printer` — Impressão do ClassFile
| Arquivo | Papel | Descrição |
|---|---|---|
| `src/lib/printer/printer.c` | Impressão do ClassFile decodificado | Implementa funções para imprimir em formato legível todas as seções do ClassFile: cabeçalho, pool de constantes, interfaces, campos, métodos e atributos. |
| `src/lib/printer/printer.h` | Interface do printer | Declara `printClassFile`, `printInterfaces`, `printFields`, `printMethods`, `printClassFileAttributes`, `printFileToTerminal`. |

---

### Módulo: `utils` — Utilitários gerais
| Arquivo | Papel | Descrição |
|---|---|---|
| `src/lib/utils/args.c` | Parser de argumentos CLI | Implementa `parse_args()` para processar `argc/argv` e `build_default_output_path()` para construir o caminho de saída padrão. |
| `src/lib/utils/args.h` | Interface de argumentos | Declara a struct `Args` e as funções de parsing. |
