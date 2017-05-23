#ifndef BLOWFISH_H
#define BLOWFISH_H

#include <stdint.h>

#include <cstddef>
#include <cstdlib>
#include <string>

#include <QByteArray>

class Blowfish {

    //Объединение, представляющее собой конвертор 32-х битового представления данных
    union Aword
    {
        uint32_t bit_32;
        struct
        {
            //Порядка байтов размещения байтов (intel)
            uint8_t byte3;
            uint8_t byte2;
            uint8_t byte1;
            uint8_t byte0;
        } bit_8;
    };

public:

    //Генерации матрицы раундовых ключей и матрицы подстановки (расширения ключа)
    void SetKey(const std::string& key);
    void SetKey(const char* key, size_t byte_length);
    
    //Интерфейс шифрования/дешифрования данных
    void Encrypt(QByteArray* dst, const QByteArray& src) const;
    void Decrypt(QByteArray* dst, const QByteArray& src) const;
    //Перегруженный вариант интерфейса шифрования/дешифрования данных
    void Encrypt(std::string* dst, const std::string& src) const;
    void Decrypt(std::string* dst, const std::string& src) const;

private:

    //Длинна блока переданных данных должан быть кратна 8 байта (64 битам)
    void Encrypt(char* dst, const char* src, size_t byte_length) const;
    void Decrypt(char* dst, const char* src, size_t byte_length) const;

    //Сеть Фейстеля
    void EncryptBlock(uint32_t* left, uint32_t* right) const;
    void DecryptBlock(uint32_t* left, uint32_t* right) const;
    //Функция итерации (раунда)
    uint32_t Feistel(uint32_t value) const;

    //Наибольший общий делитель
    unsigned long GCD(unsigned long larger, unsigned long smaller);

    //Длину блока добаленного для выравнивания исходных данных
    size_t PaddingLength(const QByteArray& data) const;
    size_t PaddingLength(const std::string& data) const;
    
private:

    uint32_t pary_[18];     //Матрица раундовых ключей
    uint32_t sbox_[4][256]; //Матрица подстановки (замены)

};

#endif /* BLOWFISH_H */
