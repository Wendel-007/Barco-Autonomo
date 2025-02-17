# Barco Autônomo para Desvio de Obstáculos🚤

Um projeto de barco autônomo que utiliza sensores ultrassônicos para detecção de obstáculos e toma decisões de navegação com base em distâncias medidas. Desenvolvido com Arduino e FreeRTOS para multitarefa.

## 📋 Visão Geral
O barco autônomo é capaz de:
- Medir distâncias em tempo real usando dois sensores ultrassônicos (esquerda e direita).
- Controlar a direção do servo motor para desviar de obstáculos.
- Ajustar a velocidade do motor com base na proximidade de obstáculos.
- Alertar visualmente (LED vermelho) e sonoramente (buzzer) em situações de risco.

## 🚀 Funcionalidades
- **Navegação autônoma**: Desvia de obstáculos de forma inteligente.
- **Multitarefa real**: Utiliza FreeRTOS para gerenciar 4 tarefas paralelas.
- **Três modos de operação**:
  - **Perigoso**: Ativa buzzer e LED quando obstáculo está a < 100 cm.
  - **Razoável**: Mantém velocidade moderada entre 100-150 cm.
  - **Seguro**: Navega em velocidade normal acima de 150 cm.

## 🛠 Componentes Hardware
| Componente          | Pino Arduino |
|---------------------|--------------|
| Sensor Ultrassônico 1 (Direita) | Trigger: 7, Echo: 5 |
| Sensor Ultrassônico 2 (Esquerda) | Trigger: 1, Echo: 0 |
| Servo Motor          | Pino 4       |
| Motor CC             | Pinos 9 e 10 |
| LED Vermelho         | Pino 12      |
| Buzzer               | Pino 13      |

## ⚙️ Configuração Software
1. **Requisitos**:
   - Arduino IDE
   - Bibliotecas: `Arduino_FreeRTOS`, `Servo`

2. **Instalação**:
   ```bash
   # No Arduino IDE:
   Sketch > Include Library > Manage Libraries...
   # Instale "FreeRTOS" e "Servo"

## 📤 Como Carregar o Código
```arduino
1. Conecte o Arduino ao computador
2. Selecione a placa correta em `Tools > Board`
3. Compile e carregue o sketch
4. Monitoramento opcional via Serial Monitor
