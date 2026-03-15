#pragma once

#include <iostream>
#include <fstream>
#include <memory>
#include <cstdint>

#include <stdexcept>


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
        size_t max_window_size = 5;

        /**
         * @brief Quantidade de janelas de bytes necessárias para cobrir o arquivo de forma completa.
         */
        size_t window_count = 0;

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
     * @brief Smart Pointer para armazenarmos os bytes.
     */
    std::unique_ptr<std::byte[]> __data;

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

        if(
            !file.is_open()
        ){
            throw std::runtime_error("Erro: Falha ao abrir o arquivo especificado.");
        }

        /* Mover o ponteiro para o final do arquivo, independente do tamanho deste. */
        this->__tfs_bytes = file.tellg();
        this->__tfs_bits = this->__tfs_bytes * 8;
        this->__bw.window_count = 1 + this->__tfs_bytes / this->__bw.max_window_size;

        /* Alocar na RAM os bytes do arquivo */
        this->__data = std::make_unique<std::byte[]>(
                                                /* Mais comum será estar dentro da Janela Máxima*/
                                                this->__bw.window_count == 1 ? this->__tfs_bytes : this->__bw.max_window_size
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
            this->__bw.window_count == 1 ? this->__tfs_bytes : this->__bw.max_window_size
        );
        if(this->__bw.window_count > 1){
            /* Apesar de ineficiente, isso tudo será executado apenas uma vez. */
            this->__bw.filepath = filepath;
        }
        file.close();
    }



};
