#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "sqlite3.h"

sqlite3 *db;

// ==================================================
//                FUNÇÕES DO BANCO
// ==================================================

void inicializar_banco() {
    int rc = sqlite3_open("ferralog.db", &db);
    if (rc != SQLITE_OK) {
        printf("Erro ao abrir banco: %s\n", sqlite3_errmsg(db));
        exit(1);
    }

    const char *sql_criar =
        "CREATE TABLE IF NOT EXISTS ferramentas ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "nome TEXT NOT NULL,"
        "codigo_qr TEXT,"
        "valor_compra REAL,"
        "possui_garantia INTEGER);";

    char *erro_msg = NULL;
    rc = sqlite3_exec(db, sql_criar, 0, 0, &erro_msg);
    if (rc != SQLITE_OK) {
        printf("Erro ao criar tabela: %s\n", erro_msg);
        sqlite3_free(erro_msg);
    }
}

void inserir_ferramenta(char *nome, char *qr, float valor, int garantia) {
    const char *sql = "INSERT INTO ferramentas (nome, codigo_qr, valor_compra, possui_garantia) "
                       "VALUES (?, ?, ?, ?);";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Erro ao preparar insert: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(stmt, 1, nome, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, qr, -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 3, valor);
    sqlite3_bind_int(stmt, 4, garantia);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        printf("Erro ao inserir: %s\n", sqlite3_errmsg(db));
    } else {
        printf("Ferramenta salva no banco com sucesso!\n");
    }

    sqlite3_finalize(stmt);
}

void listar_ferramentas() {
    const char *sql = "SELECT id, nome, codigo_qr, valor_compra, possui_garantia FROM ferramentas;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Erro ao consultar: %s\n", sqlite3_errmsg(db));
        return;
    }

    printf("%-4s %-20s %-12s %-10s %-8s\n", "ID", "Nome", "QR", "Valor", "Garantia");
    int achou = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        achou = 1;
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char *nome = sqlite3_column_text(stmt, 1);
        const unsigned char *qr = sqlite3_column_text(stmt, 2);
        double valor = sqlite3_column_double(stmt, 3);
        int garantia = sqlite3_column_int(stmt, 4);

        printf("%-4d %-20s %-12s R$%-8.2f %-8s\n",
               id, nome, qr, valor, garantia ? "SIM" : "NAO");
    }
    if (!achou) printf("Nenhuma ferramenta cadastrada.\n");

    sqlite3_finalize(stmt);
}

void buscar_por_nome(char *termo_busca) {
    const char *sql = "SELECT id, nome, codigo_qr, valor_compra, possui_garantia "
                       "FROM ferramentas WHERE nome LIKE ?;";
    sqlite3_stmt *stmt;
    char termo_like[60];
    sprintf(termo_like, "%%%s%%", termo_busca);

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Erro ao preparar busca: %s\n", sqlite3_errmsg(db));
        return;
    }
    sqlite3_bind_text(stmt, 1, termo_like, -1, SQLITE_STATIC);

    int achou = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        achou = 1;
        printf("ID: %d | Nome: %s | QR: %s | Valor: R$%.2f | Garantia: %s\n",
            sqlite3_column_int(stmt, 0),
            sqlite3_column_text(stmt, 1),
            sqlite3_column_text(stmt, 2),
            sqlite3_column_double(stmt, 3),
            sqlite3_column_int(stmt, 4) ? "SIM" : "NAO");
    }
    if (!achou) printf("Nenhuma ferramenta encontrada com esse nome.\n");

    sqlite3_finalize(stmt);
}

void remover_ferramenta(int id) {
    const char *sql = "DELETE FROM ferramentas WHERE id = ?;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Erro ao preparar remoção: %s\n", sqlite3_errmsg(db));
        return;
    }
    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        printf("Erro ao remover: %s\n", sqlite3_errmsg(db));
    } else if (sqlite3_changes(db) == 0) {
        printf("Nenhuma ferramenta com esse ID.\n");
    } else {
        printf("Ferramenta removida.\n");
    }

    sqlite3_finalize(stmt);
}

// ==================================================
//                      MAIN
// ==================================================

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    inicializar_banco();

    int opcao_menu = 0;

    // Variáveis de cadastro
    char nome_ferramenta[50];
    char codigo_qr[30];
    float valor_compra = 0.0;
    int possui_garantia = 0;

    // Loop principal do sistema
    do {
        printf("==================================================\n");
        printf("          SISTEMA FERRALOG - INVENTÁRIO            \n");
        printf("==================================================\n");
        printf("[1] Cadastrar nova ferramenta\n");
        printf("[2] Consultar todas as ferramentas\n");
        printf("[3] Relatórios\n");
        printf("[4] Sair do sistema\n");
        printf("[5] Buscar ferramenta por nome\n");
        printf("[6] Remover ferramenta\n");
        printf("==================================================\n");
        printf("Digite a opção desejada: ");
        scanf("%d", &opcao_menu);

        system("cls");

        switch (opcao_menu) {
            case 1:
                printf("--- MÓDULO DE CADASTRO ---\n");
                printf("Nome da ferramenta: ");
                scanf(" %[^\n]", nome_ferramenta);

                printf("Código QR: ");
                scanf(" %[^\n]", codigo_qr);

                printf("Valor da compra: R$ ");
                scanf("%f", &valor_compra);

                printf("Possui garantia? (1 para SIM, 0 para NÃO): ");
                scanf("%d", &possui_garantia);

                system("cls");

                if (valor_compra > 0) {
                    inserir_ferramenta(nome_ferramenta, codigo_qr, valor_compra, possui_garantia);

                    printf("--- RECIBO FERRALOG ---\n");
                    printf("Ferramenta: %s\n", nome_ferramenta);
                    printf("QR Code: %s\n", codigo_qr);
                    printf("Valor: R$ %.2f\n", valor_compra);
                    printf("Garantia: %s\n", possui_garantia ? "SIM" : "NÃO");

                    // Alerta de risco financeiro
                    if (valor_compra > 1000.0 && possui_garantia == 0) {
                        printf("\n*** ALERTA DE ALTO RISCO FINANCEIRO ***\n");
                        printf("Motivo: Equipamento de alto valor e sem cobertura de garantia.\n");
                    } else {
                        printf("\nSTATUS: Cadastro em conformidade.\n");
                    }
                } else {
                    printf("ERRO: Valor da compra inválido.\n");
                }
                printf("\n");
                break;

            case 2:
                printf("--- LISTA DE FERRAMENTAS ---\n");
                listar_ferramentas();
                printf("\n");
                break;

            case 3:
                printf("Módulo de relatórios em desenvolvimento...\n\n");
                break;

            case 4:
                printf("Salvando dados...\nSaindo do sistema.\n");
                sqlite3_close(db);
                break;

            case 5: {
                char termo[50];
                printf("Digite o nome (ou parte) da ferramenta: ");
                scanf(" %[^\n]", termo);
                buscar_por_nome(termo);
                printf("\n");
                break;
            }

            case 6: {
                int id_remover;
                printf("Digite o ID da ferramenta a remover: ");
                scanf("%d", &id_remover);
                remover_ferramenta(id_remover);
                printf("\n");
                break;
            }

            default:
                printf("ERRO: A opção %d não existe no menu.\n\n", opcao_menu);
                break;
        }

    } while (opcao_menu != 4);

    return 0;
}
