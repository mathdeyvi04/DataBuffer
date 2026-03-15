#pragma once

#include <iostream>
#include <fstream>
#include <memory>
#include <algorithm>
#include <stdexcept>

#ifdef ENABLE_BINARY_LOOKTABLE
#include <array>
#endif

class DataBuffer {
private:

    /**
     * @brief Total File Size Bytes -> Quantidade total de bytes no arquivo
     */
    size_t __tfs_bytes = 0;

    /**
     * @brief Total File Size Bits -> Quantidade total de bits no arquivo
     */
    size_t __tfs_bits = 0;

    friend struct ByteWindow;
    /**
     * @brief Struct responsável por gerenciar as janela de bytes do arquivo
     * @details
     * A intenção é que não carreguemos o arquivo completo na RAM a fim de aprimorar
     * a eficiência no uso de memória.
     */
    struct ByteWindow {
        /**
         * @brief Tamanho máximo de uma janela em Bytes
         */
        size_t max_window_size = 2;

        /**
         * @brief Quantidade de janelas de bytes necessárias para cobrir o arquivo de forma completa.
         */
        size_t window_count = 0;

        /**
         * @brief Quantidade de Bytes Utilizada Na Janela
         */
        size_t current_window_bytes = 0;

        /**
         * @brief Número da Janela Atual, é fato que sempre começaremos na primeira
         */
        size_t current_window = 1;

        /**
         * @brief Caso seja necessário o uso de janelas, precisaremos do local onde pegar as próximas.
         */
        std::string filepath = "";
    };

    /**
     * @brief Entidade ByteWindow responsável por essa instância.
     */
    ByteWindow __bw;

    /**
     * @brief Atualiza o buffer com a próxima janela de dados do arquivo.
     * @details
     * Avança para a próxima janela, lê os dados do arquivo e retorna indicador
     * de continuidade.
     * @return true  Se ainda há janelas para ler (próxima chamada terá dados).
     * @return false Se chegou ao fim do arquivo (última janela processada).
     * @note Quando retorna false, o buffer volta para a primeira janela (wrap).
     * @note O tamanho da última janela pode ser menor que max_window_size.
     */
    bool __update_window(){

        bool will_be_retorned = true;

        if(this->__bw.current_window == this->__bw.window_count){

            /* Caso haja apenas uma janela desde o início */
            if(this->__bw.window_count == 1){
                return false;
            }

            /* Demonstração de Como Modularidade é Insana */
            this->__bw.current_window = 0;
            will_be_retorned = false;
        }

        std::ifstream file(
            this->__bw.filepath,
            std::ios::binary
        );
        file.seekg(
            this->__bw.current_window * this->__bw.max_window_size,
            std::ios::beg
        );

        if(!file.is_open()){
            throw std::runtime_error("Erro: Falha ao abrir o arquivo especificado durante atualização da janela de bytes.");
        }

        this->__bw.current_window_bytes = std::min(
                                                  this->__tfs_bytes - this->__bw.current_window * this->__bw.max_window_size,
                                                  this->__bw.max_window_size
                                                  );
        file.read(
            reinterpret_cast<char*>(this->__data.get()),
            this->__bw.current_window_bytes
        );
        this->__bw.current_window++;

        file.close();
        return will_be_retorned;
    }


    /**
     * @brief Smart Pointer para armazenarmos os bytes.
     */
    std::unique_ptr<std::byte[]> __data;

#ifdef ENABLE_BINARY_LOOKTABLE
    /**
     * @brief Lookup table para conversão byte -> string binária (8 bits)
     * @details
     * Antes era necessário que calculassemos o valor de cada bit para cada byte
     * de um arquivo. Agora poderemos ver cada byte de forma individual enquanto
     * podemos ver os bits.
     *
     * Em teoria, essa função pode ser aprimorada conforme aumentarmos a quantidade
     * de bytes que podem ser traduzidos de uma vez.
     */
    static constexpr std::array<std::array<char, 8>, 256> byte2binary =
        []() constexpr {

            std::array<std::array<char, 8>, 256> table{};
            for(size_t i = 0;  i < 256; ++i){
                for(int j = 7; j >= 0; --j){
                    table[i][7 - j] = ((i >> j) & 1) ? '1' : '0';
                }
            }
            return table;
        }();
#endif

public:
    /**
     * @brief Construtor DataBuffer que carrega dados de um arquivo.
     * @details
     * Este construtor abre um arquivo binário, determina seu tamanho, aloca memória
     * para armazenar seus dados (total ou parcialmente) e realiza a leitura.
     *
     * O comportamento de alocação depende do tamanho do arquivo em relação ao
     * tamanho máximo de janela configurado:
     * - Se o arquivo for menor que a janela máxima: aloca memória para o arquivo inteiro
     * - Se o arquivo for maior: aloca apenas o tamanho da janela máxima
     * @param filepath Caminho para o arquivo a ser carregado.
     * @throws std::runtime_error Se o arquivo não puder ser aberto.
     */
    DataBuffer(const std::string& filepath){
        std::ifstream file(
            filepath,
            /* Abrir em forma binária e levar cursor ao final do arquivo */
            std::ios::binary | std::ios::ate
        );

        if(!file.is_open()){
            throw std::runtime_error("Erro: Falha ao abrir o arquivo especificado.");
        }

        /* Mover o ponteiro para o final do arquivo, independente do tamanho deste. */
        this->__tfs_bytes = file.tellg();
        this->__tfs_bits = this->__tfs_bytes * 8;
        this->__bw.window_count = 1 + this->__tfs_bytes / this->__bw.max_window_size;
        this->__bw.current_window_bytes = std::min(this->__tfs_bytes, this->__bw.max_window_size);

        /* Alocar na RAM os bytes do arquivo */
        this->__data = std::make_unique<std::byte[]>(
                                                /* Mais comum será estar dentro da Janela Máxima*/
                                                this->__bw.current_window_bytes
                                                );

        /* Mover o ponteiro de volta ao início do arquivo, independente do tamanho deste. */
        file.seekg(0, std::ios::beg);
        file.read(
            reinterpret_cast<char*>(
                this->__data.get()
            ),
            /*
            Carregar o arquivo inteiro na RAM, caso seja menor que o tamanho máximo da janela.
            Caso contrário, carrega apenas uma parte.
             */
            this->__bw.current_window_bytes
        );
        if(this->__bw.window_count > 1){
            /* Apesar de ineficiente, isso tudo será executado apenas uma vez. */
            this->__bw.filepath = filepath;
        }
        file.close();
    }

    /**
     * @brief Construtor de cópia da classe DataBuffer.
     * @details
     * Realiza cópia profunda (deep copy) dos dados do buffer.
     * @param other Buffer fonte a ser copiado.
     */
    DataBuffer(const DataBuffer& other){

        this->__tfs_bytes = other.__tfs_bytes;
        this->__tfs_bits  = other.__tfs_bits;
        this->__bw = other.__bw;

        this->__data = std::make_unique<std::byte[]>(
                                                    this->__bw.current_window_bytes
                                                    );

        std::copy(
            other.__data.get(),
            other.__data.get() + other.__bw.current_window_bytes,
            this->__data.get()
        );
    }

#ifdef ENABLE_BINARY_LOOKTABLE
    /**
     * @brief Exibe o buffer em formato binário no stream fornecido.
     * @param os Stream de saída (ex: std::cout).
     * @param only_this_window Mostrará os bits da janela atual caso verdadeiro,
     * caso contrário, percorrerá toda a estrutura a fim de apresentar todos os bits. Default: verdadeiro.
     *
     * Pode ser aprimorada utilizando-se um buffer std::string para reduzir chamadas
     * de os.write e printar diversos bytes por vez.
     */

    /**
     * @brief Exibe o buffer em formato binário no stream.
     * @details
     * Pode ser aprimorada utilizando-se um buffer std::string para reduzir chamadas
     * de os.write e printar diversos bytes por vez.
     * @tparam only_this_window Se true, mostra apenas a janela atual.
     *                          Se false, percorre todas as janelas automaticamente.
     * @param os Stream de saída (ex: std::cout).
     */
    template<bool only_this_window>
    void dump_bits(std::ostream& os) {

        const std::byte* ptr = this->__data.get();
        for(size_t i = 0; i < this->__bw.current_window_bytes; ++i){

            /* Printar 1 byte por vez para melhorar legibilidade da função */
            os.write(
                DataBuffer::byte2binary[
                    std::to_integer<unsigned char>(ptr[i])
                ].data(),
                8
            );

            if constexpr(!only_this_window){
                if((i + 1) == this->__bw.current_window_bytes){
                    if(this->__update_window()){
                        /* Pois ao final deste, ainda será somado uma unidade. */
                        i = -1;
                    }
                    else{
                        return;
                    }
                }
            }
        }
    }

#endif

};
