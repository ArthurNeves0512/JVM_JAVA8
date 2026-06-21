#include "opcodes.h"
#include "consts.h"
#include <string.h>

/*    Tabela de metadados */

const OpcodeInfo opcode_table[256] = {
    /* 0x00 */ { "nop",              1 },
    /* 0x01 */ { "aconst_null",      1 },
    /* 0x02 */ { "iconst_m1",        1 },
    /* 0x03 */ { "iconst_0",         1 },
    /* 0x04 */ { "iconst_1",         1 },
    /* 0x05 */ { "iconst_2",         1 },
    /* 0x06 */ { "iconst_3",         1 },
    /* 0x07 */ { "iconst_4",         1 },
    /* 0x08 */ { "iconst_5",         1 },
    /* 0x09 */ { "lconst_0",         1 },
    /* 0x0A */ { "lconst_1",         1 },
    /* 0x0B */ { "fconst_0",         1 },
    /* 0x0C */ { "fconst_1",         1 },
    /* 0x0D */ { "fconst_2",         1 },
    /* 0x0E */ { "dconst_0",         1 },
    /* 0x0F */ { "dconst_1",         1 },
    /* 0x10 */ { "bipush",           2 },
    /* 0x11 */ { "sipush",           3 },
    /* 0x12 */ { "ldc",              2 },
    /* 0x13 */ { "ldc_w",            3 },
    /* 0x14 */ { "ldc2_w",           3 },
    /* 0x15 */ { "iload",            2 },
    /* 0x16 */ { "lload",            2 },
    /* 0x17 */ { "fload",            2 },
    /* 0x18 */ { "dload",            2 },
    /* 0x19 */ { "aload",            2 },
    /* 0x1A */ { "iload_0",          1 },
    /* 0x1B */ { "iload_1",          1 },
    /* 0x1C */ { "iload_2",          1 },
    /* 0x1D */ { "iload_3",          1 },
    /* 0x1E */ { "lload_0",          1 },
    /* 0x1F */ { "lload_1",          1 },
    /* 0x20 */ { "lload_2",          1 },
    /* 0x21 */ { "lload_3",          1 },
    /* 0x22 */ { "fload_0",          1 },
    /* 0x23 */ { "fload_1",          1 },
    /* 0x24 */ { "fload_2",          1 },
    /* 0x25 */ { "fload_3",          1 },
    /* 0x26 */ { "dload_0",          1 },
    /* 0x27 */ { "dload_1",          1 },
    /* 0x28 */ { "dload_2",          1 },
    /* 0x29 */ { "dload_3",          1 },
    /* 0x2A */ { "aload_0",          1 },
    /* 0x2B */ { "aload_1",          1 },
    /* 0x2C */ { "aload_2",          1 },
    /* 0x2D */ { "aload_3",          1 },
    /* 0x2E */ { "iaload",           1 },
    /* 0x2F */ { "laload",           1 },
    /* 0x30 */ { "faload",           1 },
    /* 0x31 */ { "daload",           1 },
    /* 0x32 */ { "aaload",           1 },
    /* 0x33 */ { "baload",           1 },
    /* 0x34 */ { "caload",           1 },
    /* 0x35 */ { "saload",           1 },
    /* 0x36 */ { "istore",           2 },
    /* 0x37 */ { "lstore",           2 },
    /* 0x38 */ { "fstore",           2 },
    /* 0x39 */ { "dstore",           2 },
    /* 0x3A */ { "astore",           2 },
    /* 0x3B */ { "istore_0",         1 },
    /* 0x3C */ { "istore_1",         1 },
    /* 0x3D */ { "istore_2",         1 },
    /* 0x3E */ { "istore_3",         1 },
    /* 0x3F */ { "lstore_0",         1 },
    /* 0x40 */ { "lstore_1",         1 },
    /* 0x41 */ { "lstore_2",         1 },
    /* 0x42 */ { "lstore_3",         1 },
    /* 0x43 */ { "fstore_0",         1 },
    /* 0x44 */ { "fstore_1",         1 },
    /* 0x45 */ { "fstore_2",         1 },
    /* 0x46 */ { "fstore_3",         1 },
    /* 0x47 */ { "dstore_0",         1 },
    /* 0x48 */ { "dstore_1",         1 },
    /* 0x49 */ { "dstore_2",         1 },
    /* 0x4A */ { "dstore_3",         1 },
    /* 0x4B */ { "astore_0",         1 },
    /* 0x4C */ { "astore_1",         1 },
    /* 0x4D */ { "astore_2",         1 },
    /* 0x4E */ { "astore_3",         1 },
    /* 0x4F */ { "iastore",          1 },
    /* 0x50 */ { "lastore",          1 },
    /* 0x51 */ { "fastore",          1 },
    /* 0x52 */ { "dastore",          1 },
    /* 0x53 */ { "aastore",          1 },
    /* 0x54 */ { "bastore",          1 },
    /* 0x55 */ { "castore",          1 },
    /* 0x56 */ { "sastore",          1 },
    /* 0x57 */ { "pop",              1 },
    /* 0x58 */ { "pop2",             1 },
    /* 0x59 */ { "dup",              1 },
    /* 0x5A */ { "dup_x1",           1 },
    /* 0x5B */ { "dup_x2",           1 },
    /* 0x5C */ { "dup2",             1 },
    /* 0x5D */ { "dup2_x1",          1 },
    /* 0x5E */ { "dup2_x2",          1 },
    /* 0x5F */ { "swap",             1 },
    /* 0x60 */ { "iadd",             1 },
    /* 0x61 */ { "ladd",             1 },
    /* 0x62 */ { "fadd",             1 },
    /* 0x63 */ { "dadd",             1 },
    /* 0x64 */ { "isub",             1 },
    /* 0x65 */ { "lsub",             1 },
    /* 0x66 */ { "fsub",             1 },
    /* 0x67 */ { "dsub",             1 },
    /* 0x68 */ { "imul",             1 },
    /* 0x69 */ { "lmul",             1 },
    /* 0x6A */ { "fmul",             1 },
    /* 0x6B */ { "dmul",             1 },
    /* 0x6C */ { "idiv",             1 },
    /* 0x6D */ { "ldiv",             1 },
    /* 0x6E */ { "fdiv",             1 },
    /* 0x6F */ { "ddiv",             1 },
    /* 0x70 */ { "irem",             1 },
    /* 0x71 */ { "lrem",             1 },
    /* 0x72 */ { "frem",             1 },
    /* 0x73 */ { "drem",             1 },
    /* 0x74 */ { "ineg",             1 },
    /* 0x75 */ { "lneg",             1 },
    /* 0x76 */ { "fneg",             1 },
    /* 0x77 */ { "dneg",             1 },
    /* 0x78 */ { "ishl",             1 },
    /* 0x79 */ { "lshl",             1 },
    /* 0x7A */ { "ishr",             1 },
    /* 0x7B */ { "lshr",             1 },
    /* 0x7C */ { "iushr",            1 },
    /* 0x7D */ { "lushr",            1 },
    /* 0x7E */ { "iand",             1 },
    /* 0x7F */ { "land",             1 },
    /* 0x80 */ { "ior",              1 },
    /* 0x81 */ { "lor",              1 },
    /* 0x82 */ { "ixor",             1 },
    /* 0x83 */ { "lxor",             1 },
    /* 0x84 */ { "iinc",             3 },
    /* 0x85 */ { "i2l",              1 },
    /* 0x86 */ { "i2f",              1 },
    /* 0x87 */ { "i2d",              1 },
    /* 0x88 */ { "l2i",              1 },
    /* 0x89 */ { "l2f",              1 },
    /* 0x8A */ { "l2d",              1 },
    /* 0x8B */ { "f2i",              1 },
    /* 0x8C */ { "f2l",              1 },
    /* 0x8D */ { "f2d",              1 },
    /* 0x8E */ { "d2i",              1 },
    /* 0x8F */ { "d2l",              1 },
    /* 0x90 */ { "d2f",              1 },
    /* 0x91 */ { "i2b",              1 },
    /* 0x92 */ { "i2c",              1 },
    /* 0x93 */ { "i2s",              1 },
    /* 0x94 */ { "lcmp",             1 },
    /* 0x95 */ { "fcmpl",            1 },
    /* 0x96 */ { "fcmpg",            1 },
    /* 0x97 */ { "dcmpl",            1 },
    /* 0x98 */ { "dcmpg",            1 },
    /* 0x99 */ { "ifeq",             3 },
    /* 0x9A */ { "ifne",             3 },
    /* 0x9B */ { "iflt",             3 },
    /* 0x9C */ { "ifge",             3 },
    /* 0x9D */ { "ifgt",             3 },
    /* 0x9E */ { "ifle",             3 },
    /* 0x9F */ { "if_icmpeq",        3 },
    /* 0xA0 */ { "if_icmpne",        3 },
    /* 0xA1 */ { "if_icmplt",        3 },
    /* 0xA2 */ { "if_icmpge",        3 },
    /* 0xA3 */ { "if_icmpgt",        3 },
    /* 0xA4 */ { "if_icmple",        3 },
    /* 0xA5 */ { "if_acmpeq",        3 },
    /* 0xA6 */ { "if_acmpne",        3 },
    /* 0xA7 */ { "goto",             3 },
    /* 0xA8 */ { "jsr",              3 },
    /* 0xA9 */ { "ret",              2 },
    /* 0xAA */ { "tableswitch",      0 },  
    /* 0xAB */ { "lookupswitch",     0 },  
    /* 0xAC */ { "ireturn",          1 },
    /* 0xAD */ { "lreturn",          1 },
    /* 0xAE */ { "freturn",          1 },
    /* 0xAF */ { "dreturn",          1 },
    /* 0xB0 */ { "areturn",          1 },
    /* 0xB1 */ { "return",           1 },
    /* 0xB2 */ { "getstatic",        3 },
    /* 0xB3 */ { "putstatic",        3 },
    /* 0xB4 */ { "getfield",         3 },
    /* 0xB5 */ { "putfield",         3 },
    /* 0xB6 */ { "invokevirtual",    3 },
    /* 0xB7 */ { "invokespecial",    3 },
    /* 0xB8 */ { "invokestatic",     3 },
    /* 0xB9 */ { "invokeinterface",  5 },
    /* 0xBA */ { "invokedynamic",    5 },
    /* 0xBB */ { "new",              3 },
    /* 0xBC */ { "newarray",         2 },
    /* 0xBD */ { "anewarray",        3 },
    /* 0xBE */ { "arraylength",      1 },
    /* 0xBF */ { "athrow",           1 },
    /* 0xC0 */ { "checkcast",        3 },
    /* 0xC1 */ { "instanceof",       3 },
    /* 0xC2 */ { "monitorenter",     1 },
    /* 0xC3 */ { "monitorexit",      1 },
    /* 0xC4 */ { "wide",             0 },  
    /* 0xC5 */ { "multianewarray",   4 },
    /* 0xC6 */ { "ifnull",           3 },
    /* 0xC7 */ { "ifnonnull",        3 },
    /* 0xC8 */ { "goto_w",           5 },
    /* 0xC9 */ { "jsr_w",            5 },
    /* 0xCA–0xFF: reservados / não usados */
    [0xCA] = { "breakpoint",         1 },
    [0xFE] = { "impdep1",            1 },
    [0xFF] = { "impdep2",            1 },
};

/* Utilitários internos de leitura de operandos*/ 

static inline int16_t s2(const u1 *code, u4 pc) {
    return (int16_t)((code[pc] << 8) | code[pc + 1]);
}

static inline int32_t s4(const u1 *code, u4 pc) {
    return (int32_t)(
        ((u4)code[pc]     << 24) |
        ((u4)code[pc + 1] << 16) |
        ((u4)code[pc + 2] <<  8) |
         (u4)code[pc + 3]
    );
}
 //calcula opcodes de tam variavel
u4 opcode_size_variable(const u1 *code, u4 pc) {
    u1 op = code[pc];

    if (op == OP_TABLESWITCH) {
        u4 pad  = (4 - ((pc + 1) % 4)) % 4;
        u4 off  = pc + 1 + pad;
        int32_t low  = s4(code, off + 4);
        int32_t high = s4(code, off + 8);
        return 1 + pad + 12 + (high - low + 1) * 4;
    }

    if (op == OP_LOOKUPSWITCH) {
        u4 pad    = (4 - ((pc + 1) % 4)) % 4;
        u4 off    = pc + 1 + pad;
        int32_t n = s4(code, off + 4);
        return 1 + pad + 8 + n * 8;
    }

    if (op == OP_WIDE) {
        u1 next = code[pc + 1];
        return (next == OP_IINC) ? 6 : 4;
    }

    return 1; 
}


static void printCpRef(cp_info *cp, u2 idx) {
    if (!cp || idx == 0) { printf("#%hu", idx); return; }

    cp_info *entry = &cp[idx];
    printf("#%-4hu // ", idx);

    switch (entry->tag) {
        case CONSTANT_Class: {
            printf("%s", cp[entry->constant_class_info->name_index].utf8_info->bytes);
            break;
        }
        case CONSTANT_Fieldref:
        case CONSTANT_Methodref:
        case CONSTANT_InterfaceMethodref: {
            u2 ci, ni;
            if (entry->tag == CONSTANT_Fieldref) {
                ci = entry->fieldRef_info->class_index;
                ni = entry->fieldRef_info->name_and_type_index;
            } else if (entry->tag == CONSTANT_Methodref) {
                ci = entry->methodRef_info->class_index;
                ni = entry->methodRef_info->name_and_type_index;
            } else {
                ci = entry->interfaceMethod_info->class_index;
                ni = entry->interfaceMethod_info->name_and_type_index;
            }
            const u1 *class_name =
                cp[cp[ci].constant_class_info->name_index].utf8_info->bytes;
            const u1 *mname =
                cp[cp[ni].nameAndType_info->name_index].utf8_info->bytes;
            const u1 *desc  =
                cp[cp[ni].nameAndType_info->descriptor_index].utf8_info->bytes;
            printf("%s.%s:%s", class_name, mname, desc);
            break;
        }
        case CONSTANT_String:
            printf("\"%s\"",
                   cp[entry->string_info->string_index].utf8_info->bytes);
            break;
        case CONSTANT_Integer:
            printf("%d", (int32_t)entry->integer_info->bytes);
            break;
        case CONSTANT_Float: {
            float f; memcpy(&f, &entry->float_info->bytes, sizeof f);
            printf("%gf", (double)f);
            break;
        }
        case CONSTANT_Long: {
            int64_t v = ((int64_t)entry->long_info->high_bytes << 32)
                        | entry->long_info->low_bytes;
            printf("%lldL", (long long)v);
            break;
        }
        case CONSTANT_Double: {
            u8 raw = ((u8)entry->double_info->high_bytes << 32)
                     | entry->double_info->low_bytes;
            double d; memcpy(&d, &raw, sizeof d);
            printf("%g", d);
            break;
        }
        case CONSTANT_NameAndType: {
            const u1 *n = cp[entry->nameAndType_info->name_index].utf8_info->bytes;
            const u1 *d = cp[entry->nameAndType_info->descriptor_index].utf8_info->bytes;
            printf("%s:%s", n, d);
            break;
        }
        case CONSTANT_Utf8:
            printf("%s", entry->utf8_info->bytes);
            break;
        default:
            printf("(tag=%u)", entry->tag);
    }
}


void printBytecode(
    const u1 *code,
    u4        code_length,
    cp_info  *cp
) {
    u4 pc = 0;

    while (pc < code_length) {
        u1           op   = code[pc];
        const OpcodeInfo *info = &opcode_table[op];

        /* tamanho real da instrução */
        u4 size = (info->size == 0)
                  ? opcode_size_variable(code, pc)
                  : info->size;

        printf("    %4u: %-16s", pc, info->mnemonic ? info->mnemonic : "unknown");

        switch (op) {

        case OP_BIPUSH:
            printf("%d", (int8_t)code[pc + 1]);
            break;

        case OP_SIPUSH:
            printf("%d", (int)s2(code, pc + 1));
            break;

        case OP_IINC:
            printf("%u  %d", code[pc + 1], (int8_t)code[pc + 2]);
            break;

        case OP_RET:
            printf("%u", code[pc + 1]);
            break;

        case OP_NEWARRAY: {
            static const char *atype[] = {
                "","","","","boolean","char","float","double",
                "byte","short","int","long"
            };
            u1 t = code[pc + 1];
            printf("%s", (t <= 11) ? atype[t] : "?");
            break;
        }

        case OP_INVOKEINTERFACE:
            printCpRef(cp, (u2)((code[pc+1] << 8) | code[pc+2]));
            printf("  count=%u", code[pc + 3]);
            break;

        case OP_INVOKEDYNAMIC:
            printCpRef(cp, (u2)((code[pc+1] << 8) | code[pc+2]));
            break;

        case OP_MULTIANEWARRAY:
            printCpRef(cp, (u2)((code[pc+1] << 8) | code[pc+2]));
            printf("  dim=%u", code[pc + 3]);
            break;

        case OP_TABLESWITCH: {
            u4 pad  = (4 - ((pc + 1) % 4)) % 4;
            u4 off  = pc + 1 + pad;
            int32_t def  = s4(code, off);
            int32_t low  = s4(code, off + 4);
            int32_t high = s4(code, off + 8);
            printf("default=%d  low=%d  high=%d",
                   (int32_t)pc + def, low, high);
            for (int32_t k = low; k <= high; k++) {
                u4 eoff = off + 12 + (k - low) * 4;
                printf("\n              %d: %d",
                       k, (int32_t)pc + s4(code, eoff));
            }
            break;
        }

        case OP_LOOKUPSWITCH: {
            u4 pad    = (4 - ((pc + 1) % 4)) % 4;
            u4 off    = pc + 1 + pad;
            int32_t def    = s4(code, off);
            int32_t npairs = s4(code, off + 4);
            printf("default=%d  npairs=%d",
                   (int32_t)pc + def, npairs);
            for (int32_t k = 0; k < npairs; k++) {
                u4 eoff = off + 8 + k * 8;
                printf("\n              %d: %d",
                       s4(code, eoff),
                       (int32_t)pc + s4(code, eoff + 4));
            }
            break;
        }
        case OP_WIDE: {
            u1  next = code[pc + 1];
            u2  idx  = (u2)((code[pc+2] << 8) | code[pc+3]);
            const char *wname = opcode_table[next].mnemonic;
            if (next == OP_IINC)
                printf("%-10s %u  %d", wname ? wname : "?", idx, s2(code, pc+4));
            else
                printf("%-10s %u",     wname ? wname : "?", idx);
            break;
        }
        case OP_IFEQ: case OP_IFNE: case OP_IFLT:
        case OP_IFGE: case OP_IFGT: case OP_IFLE:
        case OP_IF_ICMPEQ: case OP_IF_ICMPNE:
        case OP_IF_ICMPLT: case OP_IF_ICMPGE:
        case OP_IF_ICMPGT: case OP_IF_ICMPLE:
        case OP_IF_ACMPEQ: case OP_IF_ACMPNE:
        case OP_IFNULL:    case OP_IFNONNULL:
        case OP_GOTO:      case OP_JSR:
            printf("%d", (int32_t)pc + s2(code, pc + 1));
            break;

        case OP_GOTO_W: case OP_JSR_W:
            printf("%d", (int32_t)pc + s4(code, pc + 1));
            break;

        case OP_LDC_W:  case OP_LDC2_W:
        case OP_GETSTATIC: case OP_PUTSTATIC:
        case OP_GETFIELD:  case OP_PUTFIELD:
        case OP_INVOKEVIRTUAL: case OP_INVOKESPECIAL:
        case OP_INVOKESTATIC:
        case OP_NEW:    case OP_ANEWARRAY:
        case OP_CHECKCAST: case OP_INSTANCEOF:
            printCpRef(cp, (u2)((code[pc+1] << 8) | code[pc+2]));
            break;

        case OP_LDC:
            printCpRef(cp, code[pc + 1]);
            break;

        case OP_ILOAD: case OP_LLOAD: case OP_FLOAD:
        case OP_DLOAD: case OP_ALOAD:
        case OP_ISTORE: case OP_LSTORE: case OP_FSTORE:
        case OP_DSTORE: case OP_ASTORE:
            printf("%u", code[pc + 1]);
            break;

        default:
            break;
        }

        printf("\n");
        pc += size;
    }
}