/*
   Исходный файл для создания консольного приложения, использующего библиотеки "Message-PRO".
*/

#include <stdio.h>
#include <string.h>

#include "mespro.h"

#include <iostream>
#include <fstream>
#include "ic.h"

static int Encryption(char *f, char *t, char *k) {
    void *enc_ctx = NULL;
    void *buf1 = NULL, *buf2 = NULL;
    int i = 0, ln1, ln2;
    char strbuf[1255];

    // Функции шифрования с использованием ключей СКЗИ.

    fprintf(stdout, "Encryption...\n");

    // Шифрование.

    // Считываем из СКЗИ секретный ключ автора подписи.
    // Ключи СКЗИ могут быть считаны также функциями
    // AddPSEPrivateKeyFromBuffer() и AddPSEPrivateKeys().
    strbuf[0] = 0;
    strcat(strbuf, k);
    strcat(strbuf, "pse/u1");

    if ((i = AddPSEPrivateKey(strbuf, NULL)) != 0) goto end;

    // Считываем в память сертификаты Удостоверяющего центра (УЦ)
    // для проверки сертификата автора подписи и списков отозванных сертификатов.

    strbuf[0] = 0;
    strcat(strbuf, k);
    strcat(strbuf, "pse/u1/ca");

    if ((i = AddCAs(strbuf)) != 0) goto end;

    // Создание контекста подписи.
    if ((enc_ctx = GetSignCTX()) == NULL) goto end;

    // Добавление автора подписи в контекст (задаем файл его сертификата).
    strbuf[0] = 0;
    strcat(strbuf, k);
    strcat(strbuf, "pse/u1/cert/cert1.pem");
    if ((i = AddSigner(enc_ctx, BY_FILE, strbuf,
                       NULL)) != 0)
        goto end;

    fprintf(stdout, "File encryption...\n");

    // Шифрование файла.

    if ((i = EncryptOneFile(enc_ctx, f, t)) != 0) goto end;

    fprintf(stdout, "OK\n");

    FreeSignCTX(enc_ctx);
    enc_ctx = NULL;

    fprintf(stdout, "OK\n");

    end:
    if (enc_ctx != NULL) FreeSignCTX(enc_ctx);

    return i;
}

static int ToUTF8(std::string f) {
    std::ifstream input(f);
    std::ofstream output(f + "tmp");

    for (std::string line; getline(input, line);) {
        string utf = cp2utf(line);
        output << utf << endl;
    }

    input.close();
    output.close();

    cout << f.c_str() << "\n";
    cout << (f + "tmp").c_str() << "\n";

    std::remove(f.c_str());
    cout << strerror(errno) << "\n";
    cout << std::rename((f + "tmp").c_str(), f.c_str()) << "\n";

    return 0;
}

static int Decryption(char *f, char *t, char *k) {
    void *enc_ctx = NULL;
    void *buf1 = NULL, *buf2 = NULL;
    int i = 0, ln1, ln2;
    char strbuf[1255];

    // Функции дешифрования с использованием ключей СКЗИ.

    fprintf(stdout, "Decryption... to %s\n", t);

    // Дешифрование.

    // Считываем из СКЗИ секретный ключ автора подписи.
    // Ключи СКЗИ могут быть считаны также функциями
    // AddPSEPrivateKeyFromBuffer() и AddPSEPrivateKeys().
    strbuf[0] = 0;
    strcat(strbuf, k);
    strcat(strbuf, "pse/u1");

    if ((i = AddPSEPrivateKey(strbuf, NULL)) != 0) goto end;

    // Считываем в память сертификаты Удостоверяющего центра (УЦ)
    // для проверки сертификата автора подписи и списков отозванных сертификатов.

    strbuf[0] = 0;
    strcat(strbuf, k);
    strcat(strbuf, "pse/u1/ca");

    if ((i = AddCAs(strbuf)) != 0) goto end;

    // Создание контекста подписи.
    if ((enc_ctx = GetSignCTX()) == NULL) goto end;

    // Добавление автора подписи в контекст (задаем файл его сертификата).
    strbuf[0] = 0;
    strcat(strbuf, k);
    strcat(strbuf, "pse/u1/cert/cert1.pem");
    //strcat(strbuf, "NEWKEY2021/Key#1/OpenKeys/serverprod_contact_gost2012.pem");
    if ((i = AddSigner(enc_ctx, BY_FILE, strbuf,
                       NULL)) != 0)
        goto end;

    // Считываем в память из заданного каталога сертификаты получателя
    // зашифрованных данных. Сертификат может быть считан также
    // функцией AddCertificate().
    if ((i = AddCertificate(strbuf)) != 0) goto end;

    fprintf(stdout, "File decryption...\n");

    // Дешифрование файла.

    if ((i = DecryptOneFile(f, t)) != 0) goto end;

    fprintf(stdout, "OK\n");

//    strbuf[0] = 0;
//    strcat(strbuf, k);
//    strcat(strbuf, "pse/u1/ca/ca2012.pem");
//    //strcat(strbuf, "NEWKEY2021/Key#1/OpenKeys/rarsacert_7.pem");
//    if ((i = AddSigner(enc_ctx, BY_FILE, strbuf,
//                       NULL)) != 0)
//        goto end;

    i = CheckFileSignEx(enc_ctx, t, NULL, 1, NULL);

    i = ToUTF8(t);

    fprintf(stdout, "OK\n");


    FreeSignCTX(enc_ctx);
    enc_ctx = NULL;

    fprintf(stdout, "OK\n");

    end:
    if (enc_ctx != NULL) FreeSignCTX(enc_ctx);

    return i;
}

static void SignaturesInfo(void *sgn_ctx) {
    char *sub, *cert = NULL, *oid, *value, *name = NULL;
    void *cert_ctx;
    int i, j, count, len;

    fprintf(stdout, "Signatures:\n");

    // Данная функция возвращает число подписей.
    count = GetSignatureCount(sgn_ctx);

    for (j = 0; j < count; j++) {
        // Данная функция возвращает имя владельца сертификата подписи в
        // строковом представлении.
        if ((sub = GetSignatureSubject(sgn_ctx, j)) != NULL) {
            fprintf(stdout, "%i) %s - ", j + 1, sub);

            // Данная функция возвращает статус подписи (подтверждена или нет).
            if ((i = GetSignatureStatus(sgn_ctx, j)) == 0)
                fprintf(stdout, "OK\n");
            else
                fprintf(stdout, "Error %i\n", i);

            FreeBuffer(sub);
        }

        // Альтернативный способ вывода имени владельца сертификата подписи.

        // Данная функция получает сертификат подписи.
        if (GetSignatureCertInBuffer(sgn_ctx, j, &cert, &len) == 0) {
            if ((cert_ctx = MP_GetCertificateContext(BY_BUFFER, cert, len)) != NULL) {
                if ((MP_GetCertificateContextParam(cert_ctx,
                                                   MP_CERT_SUBJECT_NAME_PARAM, 0, &name, &len)) == 0) {
                    for (j = 0; j < GetX509NameAttributeNumber(name, len); j++) {
                        if ((oid = GetX509NameAttributeOID(name, len, j)) != NULL) {
                            printf("%s=", oid);
                            FreeBuffer(oid);
                        }
                        if ((value = GetX509NameAttributeValue(name, len, j)) != NULL) {
                            printf("%s", value);
                            FreeBuffer(value);
                        }
                        printf("\n");
                    }
                    FreeBuffer(name);
                    name = NULL;
                }
                MP_FreeCertificateContext(cert_ctx);
            }
            FreeBuffer(cert);
            cert = NULL;
        }
    }
}

static int Signature(char *f, char *t, char *k) {
    void *sgn_ctx = NULL;
    int i = 0;
    char strbuf[1255];

    // Функции подписи и проверки подписи с использованием ключей СКЗИ.

    fprintf(stdout, "Signing...\n");

    // Формирование подписи.

    // Считываем из СКЗИ секретный ключ автора подписи.
    // Ключи СКЗИ могут быть считаны также функциями
    // AddPSEPrivateKeyFromBuffer() и AddPSEPrivateKeys().
    strbuf[0] = 0;
    strcat(strbuf, k);
    strcat(strbuf, "pse/u1");

    if ((i = AddPSEPrivateKey(strbuf, NULL)) != 0) goto end;

    // Считываем в память сертификаты Удостоверяющего центра (УЦ)
    // для проверки сертификата автора подписи и списков отозванных сертификатов.

    strbuf[0] = 0;
    strcat(strbuf, k);
    strcat(strbuf, "pse/u1/ca");

    if ((i = AddCAs(strbuf)) != 0) goto end;

    // Создание контекста подписи.
    if ((sgn_ctx = GetSignCTX()) == NULL) goto end;

    // Добавление автора подписи в контекст (задаем файл его сертификата).
    strbuf[0] = 0;
    strcat(strbuf, k);
    strcat(strbuf, "pse/u1/cert/cert1.pem");
    if ((i = AddSigner(sgn_ctx, BY_FILE, strbuf,
                       NULL)) != 0)
        goto end;

    fprintf(stdout, "File signing...\n");

    // Подпись файла.
    // Подпись формируется в отдельном файле.

    if ((i = SignFileEx(sgn_ctx, f, t, 1)) != 0) goto end;

    fprintf(stdout, "OK\n");

    // Освобождаем контекст подписи.
    FreeSignCTX(sgn_ctx);
    sgn_ctx = NULL;

    fprintf(stdout, "Signature checking...\n");

    // Проверка подписи.

    // Считываем в память сертификаты Удостоверяющего центра
    // для проверки сертификата автора подписи и списков отозванных сертификатов.
    strbuf[0] = 0;
    strcat(strbuf, k);
    strcat(strbuf, "pse/u1/ca");

    if ((i = AddCAs(strbuf)) != 0) goto end;


    strbuf[0] = 0;
    strcat(strbuf, k);
    strcat(strbuf, "pse/u1/crl");
    // Считываем в память списки отозванных сертификатов.
    if ((i = AddCRLs(strbuf)) != 0) goto end;

    // Создание контекста проверки подписи.
    if ((sgn_ctx = GetSignCTX()) == NULL) goto end;

    strbuf[0] = 0;
    strcat(strbuf, k);
    strcat(strbuf, "pse/u1/cert/cert1.pem");
    // Добавление автора подписи в контекст (задаем файл его сертификата).
    if ((i = AddSigner(sgn_ctx, BY_FILE, strbuf,
                       NULL)) != 0)
        goto end;

    fprintf(stdout, "File signature checking...\n");

    // Проверка подписи файла.

    if ((i = CheckFileSignEx(sgn_ctx, t, NULL,
                             0, f)) != 0)
        goto end;

    // Вывод информации о подписи.
    SignaturesInfo(sgn_ctx);

    // Освобождаем контекст проверки подписи.
    FreeSignCTX(sgn_ctx);
    sgn_ctx = NULL;

    fprintf(stdout, "OK\n");

    end:
    if (sgn_ctx != NULL) FreeSignCTX(sgn_ctx);
    return i;
}

int main(int argc, char **argv) {
    int i = 0;
    char strbuf[1255];

    SetInputCodePage(X509_NAME_ANSI);
    SetOutputCodePage(X509_NAME_ANSI);

    strbuf[0] = 0;
    strcat(strbuf, argv[3]);
    strcat(strbuf, "pse/u1");
    if ((i = PKCS7Init(strbuf, 0)) != 0) goto end;

    printf("%d\n", argc);

    if (argc == 5) {
        // Примеры использования функций шифрования и дешифрования.
        if (strcmp(argv[4], "encrypt") == 0) {
            if ((i = Encryption(argv[1], argv[2], argv[3])) != 0) goto end;
        } else {
            if ((i = Decryption(argv[1], argv[2], argv[3])) != 0) goto end;
        }
    } else {
        // Примеры использования функций формирования и проверки подписи.
        if ((i = Signature(argv[1], argv[2], argv[3])) != 0) goto end;
    }

    end:
    //PrintDetailErrors("error.log");
    // Данная функция вызывается перед завершением работы с библиотекой.
    PKCS7Final();
    if (i != 0) fprintf(stdout, "\nError %i\n", i);
    return i;
}
