#include "blowfish.h"
#include "matrixpi.h"


void Blowfish::SetKey(const std::string& key)
{
    SetKey(key.data(), key.size());
}

//Генерации матрицы раундовых ключей и матрицы подстановки (расширения ключа)
void Blowfish::SetKey(const char* key, size_t byte_length)
{
    /*
        Инициализируем матрицу раундовых ключей и матрицу подстановки
        начальными значениями, которые определены
        в виде шестнадцатеричного представления мантиссы числа PI
    */

    memcpy(pary_, MatrixPI::initial_pary, sizeof(MatrixPI::initial_pary));
    memcpy(sbox_, MatrixPI::initial_sbox, sizeof(MatrixPI::initial_sbox));
    
    //Вычисляем размер таблицы раундовых ключей и таблицы подстановки
    static const int pary_length = sizeof(pary_) / sizeof(uint32_t);
    static const int sbox_length = sizeof(sbox_) / sizeof(uint32_t);
    
    {
        //Вычисляем размер массива расширения ключа и выделяем память под массив
        unsigned long buffer_length = byte_length / GCD(byte_length, sizeof(uint32_t));
        uint32_t *key_buffer = new uint32_t[buffer_length];
        

         //Инициализируем расширенный ключ элементами входного ключа
        for (unsigned long i = 0; i < buffer_length; ++i)
        {
            Aword converter;

            converter.bit_8.byte0 = key[(i * 4) % byte_length];
            converter.bit_8.byte1 = key[(i * 4 + 1) % byte_length];
            converter.bit_8.byte2 = key[(i * 4 + 2) % byte_length];
            converter.bit_8.byte3 = key[(i * 4 + 3) % byte_length];
            
            key_buffer[i] = converter.bit_32;
        }
        
        /*
          Значение каждого раундового ключа Pn складывается по модулю 2 (XOR)
          с соответствующим элементом исходного ключа K
         */
        for (int i = 0; i < pary_length; ++i)
        {
            uint32_t key_uint32 = key_buffer[i % buffer_length];
            pary_[i] ^= key_uint32;
        }
        
        delete[] key_buffer;
    }
    
    uint32_t left  = 0x00000000;
    uint32_t right = 0x00000000;

     /*
       Вычисляем новые значения элементов матрицы раундовых ключей и
       матрицы подстановки. Для этого используем реализованный алгоритмом сети Фейстеля
     */
    for (int i = 0; i < (pary_length / 2); ++i)
    {
        EncryptBlock(&left, &right);
        
        pary_[i * 2] = left;
        pary_[i * 2 + 1] = right;
    }
    
    for (int i = 0; i < (sbox_length / 2); ++i)
    {
        EncryptBlock(&left, &right);
        
        reinterpret_cast<uint32_t*>(sbox_)[i * 2] = left;
        reinterpret_cast<uint32_t*>(sbox_)[i * 2 + 1] = right;
    }
}

void Blowfish::Encrypt(QByteArray* dst, const QByteArray& src) const
{
    QByteArray padded_data = src;
    //Выравнивание длины массива байтов до значения кратного 64
    size_t padding_length = src.length() % sizeof(uint64_t);
    if (padding_length == 0) padding_length = sizeof(uint64_t);
    for (size_t i = 0; i < padding_length; ++i) {
        padded_data += static_cast<char>(padding_length);
    }
    
    dst->resize(padded_data.size());
    //Шифрование
    Encrypt(&(*dst->data()), padded_data.data(), padded_data.size());
}

void Blowfish::Decrypt(QByteArray* dst, const QByteArray& src) const
{
    dst->resize(src.size());
    Decrypt(&(*dst->data()), src.data(), src.size());
    size_t padding_length = PaddingLength(*dst);
    dst->resize(dst->size() - padding_length);
}

void Blowfish::Encrypt(std::string* dst, const std::string& src) const
{
    std::string padded_data = src;

    size_t padding_length = src.length() % sizeof(uint64_t);
    if (padding_length == 0) padding_length = sizeof(uint64_t);
    for (size_t i = 0; i < padding_length; ++i) {
        padded_data += static_cast<char>(padding_length);
    }

    dst->resize(padded_data.size());
    Encrypt(&(*dst)[0], padded_data.data(), padded_data.size());
}

void Blowfish::Decrypt(std::string* dst, const std::string& src) const
{
    dst->resize(src.size());
    Decrypt(&(*dst)[0], src.data(), src.size());
    size_t padding_length = PaddingLength(*dst);
    dst->resize(dst->size() - padding_length);
}

void Blowfish::Encrypt(char* dst, const char* src, size_t byte_length) const
{
    if (dst != src)
        memcpy(dst, src, byte_length);
    
    for (int i = 0; i < byte_length / sizeof(uint64_t); ++i)
    {
        uint32_t* left  = &reinterpret_cast<uint32_t*>(dst)[i * 2];
        uint32_t* right = &reinterpret_cast<uint32_t*>(dst)[i * 2 + 1];
        EncryptBlock(left, right);
    }
}

void Blowfish::Decrypt(char* dst, const char* src, size_t byte_length) const
{
    if (dst != src)
        memcpy(dst, src, byte_length);
    
    for (int i = 0; i < byte_length / sizeof(uint64_t); ++i)
    {
        uint32_t* left  = &reinterpret_cast<uint32_t*>(dst)[i * 2];
        uint32_t* right = &reinterpret_cast<uint32_t*>(dst)[i * 2 + 1];
        DecryptBlock(left, right);
    }
}

//сеть Фейстеля (шифрация)
void Blowfish::EncryptBlock(uint32_t* left, uint32_t* right) const
{
    /*
     Данные разбиваются на два блока с длиной 32 бита каждый (left, right).
     Левый блок L0 изменяется функцией итерации Feistel в зависимости от ключа,
     после чего он складывается по модулю 2 (XOR) с правым блоком.
     Результат сложения присваивается новому левому блоку L1, который становится левой
     половиной входных данных для следующего раунда, а левый блок L0 присваивается
     без изменений новому правому блоку R1, который становится правой половиной.
     Операция повторяется 16 раз (16 раундов)
    */
    for (int i = 0; i < 16; ++i)
    {
        *left  ^= pary_[i];
        *right ^= Feistel(*left);
        std::swap(*left, *right);
    }
    
    std::swap(*left, *right);
    
    *right ^= pary_[16];
    *left  ^= pary_[17];
}

//сеть Фейстеля (дешифрация)
void Blowfish::DecryptBlock(uint32_t* left, uint32_t* right) const
{
    /*
       Процесс дешифрования аналогичен процессу шифрования EncryptBlock()
       за исключением того, что раундовые ключи используются в обратном порядке.
     */
    for (int i = 0; i < 16; ++i)
    {
        *left  ^= pary_[17 - i];
        *right ^= Feistel(*left);
        std::swap(*left, *right);
    }
    
    std::swap(*left, *right);
    
    *right ^= pary_[1];
    *left  ^= pary_[0];
}

//Функция итерации (раунда)
uint32_t Blowfish::Feistel(uint32_t value) const
{
    Aword converter;
    converter.bit_32 = value;

    /*
     Входящий 32-битный блок делится на четыре 8-битных блока(1 байт).
     a, b, c, d индексы массива таблицы подстановки (замены).
    */
    uint8_t a = converter.bit_8.byte0;
    uint8_t b = converter.bit_8.byte1;
    uint8_t c = converter.bit_8.byte2;
    uint8_t d = converter.bit_8.byte3;
    
    return ((sbox_[0][a] + sbox_[1][b]) ^ sbox_[2][c]) + sbox_[3][d];
}

//Наибольший общий делитель
unsigned long Blowfish::GCD(unsigned long larger, unsigned long smaller)
{
    unsigned long gcd = smaller;
    unsigned long gcd_prev = larger;
    unsigned long gcd_next;

    while ((gcd_next = gcd_prev % gcd) != 0){
        gcd_prev = gcd;
        gcd = gcd_next;
    }

    return gcd;
}

/*
 Возвращает длину блока добавленного перед шифрованием к исходным данным,
 в случае если размер переданных исходных данных не кратен 64 битам (8 байт)
*/
size_t Blowfish::PaddingLength(const QByteArray& data) const
{
    if (data.isEmpty()) return 0;
    char length = data[data.size() - 1];
    if (length > 0 && length <= 8) {
        for (size_t i = 0; i < length; ++i) {
            if (length != data[data.size() - i - 1]) {
                return 0;
            }
        }
    }
    else
        return 0;

    return length;
}

size_t Blowfish::PaddingLength(const std::string& data) const
{
    if (data.empty()) return 0;
    char length = data[data.size() - 1];
    if (length > 0 && length <= 8) {
        for (size_t i = 0; i < length; ++i) {
            if (length != data[data.size() - i - 1]) {
                return 0;
            }
        }
    }
    else
        return 0;

    return length;
}




