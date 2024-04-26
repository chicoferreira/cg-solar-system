#import "template.typ": *

#show: project.with(
  title: "Computação Gráfica - Fase 3",
  date: "25 de Abril de 2024",
  group: [*Grupo 2*],
  authors: (
    (name: "Daniel Pereira", number: "A100545"),
    (name: "Duarte Ribeiro", number: "A100764"),
    (name: "Francisco Ferreira", number: "A100660"),
    (name: "Rui Lopes", number: "A100643"),
  ),
)

#heading(numbering: none)[Introdução]

Este relatório descreve a terceira fase do projeto de Computação Gráfica. Nesta fase foram implementadas novas funcionalidades, tais como as transformações temporais - no caso específico da translação, com recurso a curvas de _Catmull Rom_. Foi também implementado o suporte à geração de modelos com _VBOs_ com índices, melhorando, assim, a performance da renderização de modelos mais complexos. Por fim, foi implementado o suporte à geração de modelos mais complexos, em termos geométricos, com recurso a _Bezier Patches_.

= Simulação de tempo

Com grande foco desta fase, uma componente temporal teve que ser adicionada ao projeto. Esta é responsável por simular o tempo passado, permitindo a implementação das várias funcionalidades temporais que foram pedidas.

Como já tinhamos implementado o processamento do tempo na fase anterior para as acelerações e desacelerações da camera, foi relativamente simples adicionar a componente temporal ao projeto.

Agora, a `Engine` guarda também o tempo atual, que é atualizado a cada _frame_. A cada _frame_ é somado ao tempo atual o tempo que passou desde o último _frame_, permitindo assim a simulação do tempo. Com esta abordagem também é possível controlar a velocidade do tempo através de um fator de escala e também pausá-lo.

#align(center, ```cpp
void Engine::Run()
{
    float currentTime = glfwGetTime();
    while (!glfwWindowShouldClose(m_window))
    {
        ...
        const float newTime = glfwGetTime();
        const float timestep = newTime - currentTime;
        ...
        currentTime = newTime;
        m_simulation_time += timestep;
        Render();
        ...
    }
}
```)

== Integração no ImGui

Desta forma, foi adicionado uma aba no _ImGui_ @imgui que permite fazer a gestão do tempo. A aba de simulação tem um _slider_ que permite controlar a velocidade do tempo e um _checkbox_ que permite pausar o tempo.

#figure(image("fase3/simulation imgui.png"), caption: [Aba de Simulação no _ImGui_])

= Novas transformações

Com a simulação de tempo implementada, podemos passar agora às novas transformações temporais. Para isto, uma pequena restruturação no código de transformações foi necessária. Agora, a matriz resultado de cada transformação é dependente do tempo e não será uma matriz fixa. Para simplificação de implementação, a matriz de transformação não é mais guardada e alterada só quando necessário, mas sim calculada a cada _frame_. #footnote[Como os cálculos para as transformações não são muito complexos, a performance não é afetada.]

== Rotação temporal

Uma das novas transformações adicionada foi a rotação temporal. Esta permite que um grupo seja transformado de forma a que este rode em torno de um eixo (arbitrário) com que faça uma rotação completa em um determinado intervalo de tempo.

A implementação desta transformação é simples, pelo que, foi apenas necessário utilizar a matriz de rotação com um ângulo que é calculado com base no tempo:

$
cal(R_T)(t, Delta t,x,y,z) = cal(R)((2 pi t)/(Delta t), x , y,z) 
$

Sendo que $Delta t$ é o tempo que demora a completar uma rotação completa, $t$ é o tempo atual, $x$, $y$ e $z$ os eixos de rotação, e $cal(R)$ a função que calcula a matriz de rotação (enunciada na fase 2).

=== Integração no ImGui

Como regra geral deste projeto, esta transformação também foi adicionada no _ImGui_, onde é possível controlar o tempo que demora a completar uma rotação completa e o seu eixo de rotação.

#figure(image("fase3/temporal rotation imgui.png"), caption: [Rotação temporal no _ImGui_])

== Translação temporal (Catmull Rom)

A outra transformação temporal adicionada foi a translação temporal entre pontos. Esta permite que um grupo se mova entre vários pontos de controlo, de forma a que a sua trajetória seja suave e contínua.

Para a implementação desta transformação, foi utilizado o algoritmo de Catmull Rom para a posição de um ponto entre 4 pontos de controlo. Caso hajam mais de 4 pontos de controlo, a trajetória é calculada a partir dos 4 pontos de controlo mais próximos do ponto do tempo atual.

O ponto final e a sua derivada é calculado a partir das seguintes fórmulas:

$
cal(C)(P_0, P_1, P_2, P_3) = mat(
  -0.5, 1.5, -1.5, 0.5 ;
  1.0, -2.5, 2.0, -0.5 ;
  -0.5, 0.0, 0.5, 0.0 ; 
  0.0, 1.0, 0.0, 0.0
  ) mat(P_0; P_1; P_2; P_3) \ 

cal(P)(t, P_0, P_1, P_2, P_3) = cal(C)(P_0, P_1, P_2, P_3) mat(t^3, t^2, t, 1) \
cal(P')(t, P_0, P_1, P_2, P_3) = cal(C)(P_0, P_1, P_2, P_3) mat(3t^2, 2t, 1, 0) \
$

Sendo $P_0, P_1, P_2, P_3$ os pontos de controlo, $t in [0.0,1.0]$ o tempo atual, $cal(P)$ a posição atual e $cal(P')$ a derivada nessa posição.

Com a posição calculada, o resultado da transformação será apenas uma matriz de translação para esse ponto.

Esta transformação também tem um parâmetro opcional de alinhamento à curva, que usa a derivada calculada para esse efeito.

$
arrow(X_i) = norm(cal(P')(t, P_0, P_1, P_2, P_3)) \
arrow(Z_i) = norm(arrow(X_i) times arrow(Y_(i-1))) \
arrow(Y_i) = norm(arrow(Z_i) times arrow(X_i)) \
M = mat(
  arrow(X_i)_x, arrow(Y_i)_x, arrow(Z_i)_x, 0;
  arrow(X_i)_y, arrow(Y_i)_y, arrow(Z_i)_y, 0;
  arrow(X_i)_z, arrow(Y_i)_z, arrow(Z_i)_z, 0;
  0, 0, 0, 1
)
$

Sendo $i$ a iteração atual e $i-1$ a iteração anterior #footnote[Na primeira iteração, quando $i=0$, $arrow(Y_(i-1)) = (0,1,0)$.].

Esta matriz $M$ é multiplicada com a matriz de translação calculada, para a finalidade de rotação de alinhamento à curva do caminho caso o parâmetro esteja ligado.

A transposta da matriz resultado é enviada para o _OpenGL_ @opengl, usando o `glMultiMatrixf()`, completando assim a transformação.

=== Mostrar caminho da curva <path>

Para facilitar a visualização do comportamento da translação temporal, também foi adicionado a renderização do caminho da curva de Catmull Rom. Este caminho é calculado a partir das fórmulas anteriores.

#figure(image("fase3/catmull rom path.png", width: 100%), caption: [Caminho da curva de Catmull Rom])

O caminho é formado por pontos #footnote[Para já são 100 pontos, um valor não configurável.] ligados entre si, espaçados uniformemente ao longo da curva. Estes pontos são enviados para o _OpenGL_ e são renderizados como `GL_LINE_LOOP`. A forma de como são enviados para o _OpenGL_ será explicada num #link(<path_vbo>)[capítulo posterior].

=== Integração no ImGui

Esta transformação também é completamente editável em tempo real no _ImGui_.

#figure(image("fase3/catmull rom imgui.png"), caption: [Transformação temporal no _ImGui_])

Como se pode ver, é possível adicionar pontos, remover pontos, esconder/mostrar o caminho para cada transformação individualmente (também é possível esconder globalmente), trocar entre alinhar e não alinhar o modelo ao caminho e alterar o tempo do trajeto completo.

Caso a transformação tenha menos de 4 pontos de controlo, uma mensagem de aviso é mostrada ao utilizador que a transformação não tem efeito até adicionar pelo menos 4 pontos necessários.

#figure(image("fase3/catmull rom imgui error.png"), caption: [Transformação temporal no _ImGui_ com menos de 4 pontos de controlo])

= _VBOs_ com Índices

Devido ao grande número de vértices que a cena do sistema solar tem vindo a ter, a renderização no modo imediato estava a tornar-se um fator limitante. Com isto, foi implementado o uso de _VBOs_ com índices para a renderização dos modelos.

Contudo, para esse efeito, a sua implementação não foi trivial visto que os modelos não tinham informação de índices. A solução que o grupo optou foi alterar o formato de ficheiros _.3d_ e a implementação da geração desses para incluir a informação dos índices.

O novo formato de ficheiros _.3d_ agora inclui o número de vértices e o número de índices no início do ficheiro, e de seguida a lista de vértices e a lista de índices.

#figure(```
4 6

-1 0 -1
1 0 -1
-1 0 1
1 0 1

0 1 2
2 1 3
```, supplement: [Figura], caption: [Exemplo de um ficheiro .3d com índices])

Os índices são (opcionalmente) agrupados por triângulo em cada linha para melhor visualização.

Na próxima fase este formato terá que ser alterado novamente para guardar informações de normais. A geração dos modelos já foi implementada para fácil adição das normais.

A geração de modelos teve grande consideração na poupança de pontos, evitando duplicações de vértices onde não é necessário.

Na _engine_, o modelo agora é representado por uma lista de vértices e uma lista de índices. Os vértices e índices são carregados para _buffers_ da GPU e são renderizados posteriormente com `glDrawElements()`.

```cpp
void Engine::renderModel(uint32_t model_index, size_t index_count)
{
    glBindBuffer(GL_ARRAY_BUFFER, m_models_vertex_buffers[model_index]);
    glVertexPointer(3, GL_FLOAT, 0, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_models_index_buffers[model_index]);
    glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
}
```

== Considerações nas gerações

De modo geral, a novas gerações de modelos passam de uma primeira fase de cálculo de vértices, e depois de todos os vértices calculados, são calculados os índices a partir das posições dos vértices.

Isto vai assegurar que na próxima fase, bastará adicionar a geração de normais na mesma fase de cálculo de vértices.

=== Do Plano

O plano, por ser o tipo de geometria mais simples, também tem o processo de nova geração mais simples. O cálculo de índices passa simplesmente por fazer com que pontos fora das bordas apontem para o mesmo vértice, que é partilhado entre vários triângulos.

#import "fase1/3d.typ": *

#let generate_plane(length: 1, divisions: 1, iteration: 0) = {
  let side = length / divisions

  let size = 200pt
  let h_size = 150pt

  let origin = (size / 2, size / 2)
  
  box(width: size, height: h_size, {
    place(line(stroke: red, start: origin, end: project(1,0,0, size)))
    place(line(stroke: green, start: origin, end: project(0,1,0, size)))
    place(line(stroke: blue, start: origin, end: project(0,0,1, size)))

    let points = ()

    for x in range(0, divisions) {
      for z in range(0, divisions) {
        let top_left = project(-length / 2 + x * side, 0, -length / 2 + z * side, size);
        let top_right = project(-length / 2 + (x + 1) * side, 0, -length / 2 + z * side, size);
        let bottom_left = project(-length / 2 + x * side, 0, -length / 2 + (z + 1) * side, size);
        let bottom_right = project(-length / 2 + (x + 1) * side, 0, -length / 2 + (z + 1) * side, size);

        set line(stroke: gray.darken(20%))

        place(line(start: top_left, end: bottom_left))
        place(line(start: bottom_left, end: bottom_right))
        place(line(start: top_left, end: top_right))
        place(line(start: top_right, end: bottom_right))
        place(line(start: top_left, end: bottom_right))

        if(x * divisions + z == iteration) {
          points += (top_left, top_right, bottom_left, bottom_right)
        }
      }
    }

    for (i, p) in points.enumerate() {
      let x = p.at(0)
      let y = p.at(1)

      point(..p)
      
      place(dx: p.at(0), dy: p.at(1) - 13pt, $P$)
    }

    let origin = point(..project(0,0,0, size))
    origin
  })
}

#figure(
  generate_plane(length: 2, divisions: 3, iteration: 4),
  caption: [Pontos partilhados na geração de um plano]
)

=== Da Esfera
A geração da esfera agora com índices passou a ser uma das mais complicadas. A nova geração de índices agora tem em conta que vértices do topo e do chão da esfera são partilhados entre todos os _slices_ e, ao fim de uma volta completa, os pontos da _slice_ atual são agrupados #footnote[Por agrupado, entende-se que os índices que formam o triângulo correspondente partilham o mesmo vértice.] com a _slice_ inicial. Com isto, nenhum vértice está duplicado.

Também tem em consideração que na primeira e na última iteração da _stack_ só adiciona um triângulo em vez de dois, como foi enunciado no relatório da primeira fase, no capítulo *Problema dos polos da esfera*.

=== Da Caixa

Usando a estratégia de translações de matrizes a partir da geração do plano, a geração da caixa manteu-se práticamente inalterada, já que a lógica de geração dos pontos está toda na geração do plano. Só tem em consideração de *não* agrupar os pontos adjacentes entre os planos (as arestas do caixa) visto que esses terão normais diferentes.

=== Do Cilindro

No cilindro é onde tem mais duplicações de vértices. Como os pontos que unem as bases às laterais terão duas normais diferentes (uma de uma das base e uma da lateral), esses pontos tiveram que ser duplicados. Mesmo assim, o vértice do centro do topo e do centro da base são apenas adicionados uma vez, e vértices das laterais são reutilizados quando formam triângulos adjacentes. Também como na esfera, na geração ao fim da volta completa os pontos do último _slice_ são agrupados com o primeiro _slice_.

=== Do Cone

O cone segue uma união da lógica do cilindro e da esfera. Tem em conta o agrupamento de vértices no fim da volta completa, do centro da base e do topo do cone, duplicação de pontos na união da lateral e da base devido às normais, e geração de apenas um triângulo na última _stack_ de cada _slice_.

== Suporte a modelos OBJ

O suporte a modelos _Wavefront OBJ_ foi continuado. Agora, ao carregar um modelo desse tipo, os índices são usados individualmente e devidamente. Melhorias de desempenho foram notadas em modelos mais pesados.

== Caminho de translação temporal com _VBOs_ <path_vbo>

Como enunciado num #link(<path>)[capítulo anterior], a translação temporal tem a funcionalidade de mostrar o caminho por onde o trajeto da translação acontece. Com grande enfâse na performance desta fase e a possibilidade de haver uma grande quantidade de planetas/luas com esta transformação no sistema solar, se tais pontos do trajeto forem enviados em modo imediato poderão trazer grande perda de desempenho.

Com isto, cada transformação destas tem o seu _buffer_ na GPU para onde são enviados tais pontos quando há alguma alteração nos parâmetros da transformação. A sua renderização, em semelhança aos modelos, é feita com uso de _VBOs_ (sem índices visto a estar ser renderizado no modo `GL_LINE_LOOP`).

== Nova visualização de modelos no _ImGui_

No _ImGui_, a visualização de modelos também foi alterada para acomodar o novo formato. Agora uma tabela de vértices e uma tabela de triângulos (índices agrupados de 3 em 3) são mostrados, tal como o número total de vértices, índices e triângulos do modelo. 

#figure(image("fase3/model view imgui.png", width: 70%), caption: [Tabela de vértices e triângulos do modelo no _ImGui_])

== Diferenças de performance (_Benchmarks_)

Para comparar diferenças da implementação de renderização imediata em comparação com a renderização com _VBOs_ com índices, foi escolhido o sistema solar da fase anterior. Para referência esta cena tem 149940 triângulos de 181 modelos e foi renderizada numa resolução de 2560$times$1369.

Não foi possível escolher uma cena mais recente, visto que os modelos sofreram alterações nos índices, pelo que, por exemplo, não temos desenvolvido geração de modelos sem índices para _patches_ de _Bezier_.

#figure(grid(columns: 2, column-gutter: 1em, align: center + horizon,
  table(columns: 2, align: center + horizon, 
    [Sistema solar], [Performance],
    [Sem _VBOs_], [1.254 ms/frame $<->$ 797.6 FPS],
    [Com _VBOs_ com índices], [0.314ms/frame $<->$ 2930.2 FPS]
  ),

  table(columns: 2, align: center + horizon,
    table.cell(colspan: 2)[Ambiente de testes],
    [GPU], [RTX 2060 SUPER],
    [CPU], [Ryzen 7700x],
    [RAM], [DDR5 6000MHz]
  )
), caption: [Resultados do _Benckmark_])

Acreditamos que a diferença seria ainda maior caso fosse usado a cena do sistema solar desta fase, que tem 432 modelos com um total de 218400 triângulos, cuja performance no mesmo ambiente ronda os 1900FPS.

= _Bezier Patches_

Nesta fase foi implementada a capacidade do programa _generator_ de gerar modelos a partir de _Bezier Patches_. Estas _Bezier Patches_ encontram-se num ficheiro _.patch_ e seguem o formato enunciado pela equipa docente.

De forma análoga às fases anteriores, mas neste caso específico, basta correr o _generator_ da seguinte forma: `./generator patch <ficheiro.patch> <tesselation level> <output.3d>`. O segundo parâmetro, a tesselação, consiste no número de divisões que cada _Bezier Patch_ terá, ou seja, quanto maior o número, mais detalhado será o modelo. No entanto, é importante denotar que um número muito elevado #footnote[Um número acima de 10, por exemplo.] irá levar a um número de vértices gerados muito elevado, o que levaria a que a renderização de tal modelo fosse pesada desnecessariamente.

== Estrutura de um ficheiro _.patch_

#figure(```
2
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
3, 16, 17, 18, 7, 19, 20, 21, 11, 22, 23, 24, 15, 25, 26, 27
28
1.4, 0, 2.4
1.4, -0.784, 2.4
0.784, -1.4, 2.4
...
0, -1.4375, 2.53125
1.5, 0, 2.4
1.5, -0.84, 2.4
```, supplement: [Figura], caption: [Exemplo de um ficheiro .patch])

A primeira linha do ficheiro indica o número de total de _Bezier Patches_. De seguida, cada linha indica os índices dos pontos de controlo que formam a _Bezier Patch_. A linha imediatamente a seguir a todas essas indica o número de pontos de controlo. Por fim, cada uma das linhas seguintes indica as coordenadas dos pontos de controlo.

Assim, o _parser_ para ler um ficheiro _.patch_ é relativamente simples, visto que o mesmo segue um formato bem definido.

== Geração de modelos

A geração de modelos dá-se ao nível do _patch_, onde, para cada um dos _patches_ computamos os seus pontos a partir da seguinte fórmula:

$
cal(P)(u, v) =
  mat(u^3, u^2, u, 1)
  M
  mat(
    P_00, P_01, P_02, P_03 ;
    P_10, P_11, P_12, P_13 ;
    P_20, P_21, P_22, P_23 ;
    P_30, P_31, P_32, P_33
  )
  M^T
  mat(
    v^3 ;
    v^2 ;
    v ;
    1
  )
$

Sendo $P_"ij"$ os pontos de controlo do dado _patch_, $u$, $v in [0, 1]$ e

$
M = M^T = mat(
  -1, 3, -3, 1 ;
  3, -6, 3, 0 ;
  -3, 3, 0, 0 ;
  1, 0, 0, 0
)
$

Esta matriz, $M$, é derivada a partir dos coeficientes dos polinómios de _Bersntein_#footnote[Estes coeficientes podem ser derivados, por exemplo, a partir do Triângulo de Pascal.]. Neste caso, os polinómios de grau 3 , visto que estamos a lidar com curvas com 4 pontos de controlo cada.

De notar que, uma vez que $u$, $v in [0, 1]$, a geração de pontos é feita para todos os valores das ditas variáveis com passo de $1/"tesselation"$.

De notar também que, o cálculo $M A M^T$, onde $A$ é a matriz dos pontos de controlo, é feito apenas uma vez para cada _patch_, visto que o mesmo é sempre constante.

Finalmente, após os pontos estarem todos computados, o problema reduz-se a agrupá-los de forma a formarem triângulos. O grupo escolheu, primeiramente, agrupá-los em retângulos e depois dividir esses retângulos em triângulos.

Essencialmente, por cada _patch_ um ponto irá formar um retângulo com os seus vizinhos diretos, tal como desmonstra a figura seguinte. Se indexarmos os pontos com base nos valores de $u$ e $v$ usados para os gerar, um dado ponto $P_"ij"$ será agrupado com os pontos $P_"(i+1)j"$, $P_"i(j+1)"$ e $P_"(i+1)(j+1)"$.

#let generate_plane(length: 1, divisions: 1, iteration: 0) = {
  let side = length / divisions

  let size = 200pt
  let h_size = 150pt

  let origin = (size / 2, size / 2)
 
  box(width: size, height: h_size, {
    let points = ()

    for x in range(0, divisions) {
      for z in range(0, divisions) {
        let y(o) = if x + o == 0 { 0 } else if x + o == 1 or x + o == 2 { 0.2 } else { 0 }

        let top_left = project(-length / 2 + x * side, y(0), -length / 2 + z * side, size);
        let top_right = project(-length / 2 + (x + 1) * side, y(1), -length / 2 + z * side, size);
        let bottom_left = project(-length / 2 + x * side, y(0), -length / 2 + (z + 1) * side, size);
        let bottom_right = project(-length / 2 + (x + 1) * side, y(1), -length / 2 + (z + 1) * side, size);

        set line(stroke: gray.darken(20%))

        place(line(start: top_left, end: bottom_left))
        place(line(start: bottom_left, end: bottom_right))
        place(line(start: top_left, end: top_right))
        place(line(start: top_right, end: bottom_right))
        place(line(start: top_left, end: bottom_right))

        if(x == iteration and z == 0) {
          points += (top_left, top_right, bottom_left, bottom_right)
        }
      }
    }

    for (i, p) in points.enumerate() {
      let x = p.at(0)
      let y = p.at(1)

      point(..p)
 
      place(dx: p.at(0), dy: p.at(1) - 11pt, [$P_#(i + 1)$])
    }
  })
}

#figure(generate_plane(length: 1.4, divisions: 3, iteration: 1), caption: [Exemplificação do agrupamento dos pontos em triângulos])

Neste caso, $P_1$ será gerado com $u=0.333$ e $v=0$, $P_2$ com $u=0.666$ e $v=0$, $P_3$ com $u=0.333$ e $v=0.333$ e $P_4$ com $u=0.666$ e $v=0.333$.

O processo de indexação de vértices passa essencialmente no mesmo que a geração do plano.

= Sistema Solar com rotações temporais e asteroides

Agora que temos a capacidade de gerar modelos a partir de _Bezier Patches_, de renderizar grandes quantidades de vértices devido à implementação de _VBOs_ com índices e de aplicar transformações temporais, podemos finalmente dar vida ao sistema solar.

== Adição do tempo

Pelo _dataset_ dos planetas e satelites do sistema solar usado na fase anterior, já temos a informação necessária para tempos de rotações e translações de todos os corpos celestes.

Agora, com a capacidade de simular o tempo, podemos aplicar rotações e translações de forma a que os corpos celestes se movam e rodem de acordo com o tempo.

=== Rotação dos planetas

Para simular a rotação do planeta em torno do seu eixo, basta aplicar uma transformação de rotação temporal com o tempo de rotação do planeta.

Este tempo tem que ser convertido para um tempo que seja visualmente agradavel, visto que o tempo de rotação real de um planeta é muito grande.

A função que foi usada para mapear o tempo real para o tempo visual foi a seguinte:

$
Delta r_"visual" = 5log_2(Delta  r_"real")
$

Sendo $Delta r_"visual"$ o tempo de rotação visual, $Delta r_"real"$ o tempo de rotação real e $log_2$ o logaritmo #footnote[Caso $Delta  r_"real"$ for negativo, i.e. o planeta roda no sentido contrário, $Delta r_"visual"$ será negativo.] na base 2.

Esta função tem a propriedade logaritmica que ajuda a que os planetas que rodam exponecialmente mais devagar (por exemplo Venus com um dia a durar 5832 horas) tenham a sua rotação minimamente visível, apesar de mais lenta.

Por fim, é adicionada a transformação de rotação temporal no eixo $(0,1,0)$.

=== Ângulo de inclinação de planetas

De forma a ainda tornar mais real a apresentação, os planetas também são rodados ligeiramente de acordo com o seu ângulo de inclinação que também consta no _dataset_.

Esta rotação é estática e feita no eixo $(1,0,0)$.

=== Translação dos planetas em torno do sol

A translação dos planetas em torno do sol é o efeito que mais se destaca no sistema solar. Para tal, é aplicada uma translação temporal com o tempo de translação do planeta, usando a mesma fórmula de mapeamento de tempo que a rotação.

$
Delta t_"visual" = 5log_2(Delta  t_"real")
$


Como a translação necessita de pontos de controlo, estes são calculados a partir de uma circunferencia com o raio da distância do planeta ao sol. A partir daí, são retirados pontos de controlo igualmente espaçados ao longo da circunferência. Para já o número de pontos está fixo a 10, que já dá uma translação que parece ter a trajetória de uma circunferência.

Esta translação podia ser feita através de uma rotação temporal, mas a translação faz com que seja possível que o planeta tenha caminhos de translação mais complexos, como por exemplo, translações elipticas, que pretendemos implementar na próxima fase.

=== Transformações nos satelites

Os satélites do sistema solar também têm translações e rotações em torno dos seus planetas. Estas são feitas de forma análoga às dos planetas, mas com tempos diferentes. Como o _dataset_ não tem informação sobre esses tempos, estes foram gerados a partir do tempo de translação do planeta: 

$
Delta t_"satelite" = (2 Delta t_"visual") / 5
$

$
Delta r_"satelite" = (Delta t_"visual") / 5
$

Sendo $Delta t_"satelite"$ o tempo de translação à volta do planeta do satélite, $Delta t_"visual"$ o tempo da translação do planeta à volta do Sol e $Delta r_"satelite"$ o tempo de rotação do satélite.

Com estes parâmetros conseguimos fazer com que os satélites tenham movimento visualmente apelativo em relação aos planetas.

== Cintura de Asteroides

Baseado na cintura de asteroides do sistema solar, foi adicionado um grupo de asteroides que se movem em torno do sol. Estes asteroides têm translações elípticas, que são feitas a partir de uma translação temporal similarmente à dos planetas.

Cada asteroide tem a sua própria translação, com um tempo aleatório entre 30 e 35 segundos e uma posição aleatória na cintura. Esta posição pode variar em todos os eixos.

Para dar uso à performance adquirida com a implementação de _VBOs_ com índices, os asteroides são gerados a partir do patch do _teapot_. Ou seja, um _teapot_ é um asteroide. O número de asteroides é configurável na geração do sistema solar, mas como versão final desta fase, o número de asteroides é de 100. Isto é, 100 _teapots_ a moverem-se em torno do sol, uma cintura de _teapots_ portanto.

Estes _teapots_ são gerados com uma tesselação mínima de 1 para que a renderização seja rápida. Visualmente não se nota a diferença visto que os asteroides são pequenos em relação aos tamanhos dos planetas.

#figure(image("fase3/asteroid ring.png"), caption: [Cintura de asteroides])

#figure(image("fase3/asteroid ring with path.png"), caption: [Cintura de asteroides com trajetória de transformações temporais])

== Cometa

Como requisito deste enunciado, também foi adicionado um cometa que percorre uma trajetória eliptica entre perto de Marte e perto da trajetória de Urano. Os pontos da translação do cometa foram calculados de forma semelhante aos planetas, com também 10 pontos de controlo, mas esses pontos foram calculados de forma a que o cometa tenha uma trajetória elíptica usando as fórmulas de uma elipse. Numa próxima fase pretendemos expandir as trajetórias elipticas para os planetas também.

Este cometa tem como modelo também um _teapot_ mas com mais tesselação, 5, já que o seu tamanho é maior.

O grupo do cometa tem então como transformações:
- Translação no eixo $x$ para ficar descentralizado com o sistema solar
- Translação temporal com a trajetória elíptica
- Rotação de $-pi/2$ no eixo $(1,0,0)$ para o _teapot_ ficar perpendicular ao plano da elipse
- Rotação temporal de 20 segundos no eixo $(1,0,0)$ para dar o efeito de rotação ligeira do cometa em torno de si mesmo.

#figure(image("fase3/comet.png", width: 75%), caption: [Cometa _teapot_])

#figure(image("fase3/comet top down view.png", width: 75%), caption: [Cometa _teapot_ com vista de cima])

== Nomes dos corpos celestes

Para facilitar a identificação dos corpos celestes, foi adicionado um parâmetro de nome nos grupos. Este parâmetro é opcional.

#align(center,
```xml
<group name="Planet Earth">
    ...
</group>
```)

#figure(image("fase3/group name imgui.png", width: 65%), caption: [Nomes de grupos no _ImGui_])

== Estrutura final do sistema solar

Com isto o novo sistema solar pode ser gerado e ele tem a seguinte estrutura:

- Grupo Principal
  - Sol
    - Escala (para o seu tamanho)
    - Rotação (para a sua inclinação)
    - Rotação temporal (para a rotação em torno do seu eixo)
  - Grupo Planetário 1
    - Translação temporal (para o movimento de translação no sistema solar)
    - Rotação (para a sua inclinação)
    - Grupo do Planeta
      - Escala (para o seu tamanho)
      - Rotação temporal (para a rotação em torno do seu eixo)
    - Grupo do Satélite 1
      - Translação temporal (para o movimento de translação em torno do planeta)
      - Escala (para o seu tamanho)
      - Rotação temporal (para a rotação em torno do seu eixo)
  - Grupo de Asteróides
    - Asteróide 1
      - Translação temporal (para o movimento em torno da cintura)
      - Rotação de 90º no eixo $x$ (para alinhar o _teapot_ horizontalmente)
      - Escala (para o tamanho de um asteróide)
  - Grupo do Cometa
    - Translação (para deslocamento fora do centro)
    - Translação temporal (para seguir o seu trajeto)
    - Rotação (para alinhar o _teapot_ horizontalmente)
    - Rotação temporal (para rotação em torno de si mesmo)

#figure(image("fase3/solar system with paths.png"), caption: [Sistema solar com trajetória de transformações temporais])

#figure(image("fase3/solar system.png"), caption: [Sistema solar final da terceira fase])

#heading(numbering: none)[Conclusão]

De um modo geral, o projeto está a avançar de acordo com o planeado. A implementação das transformações temporais, a geração de modelos com _VBOs_ com índices e a geração de modelos com _Bezier Patches_ foram implementadas com sucesso. A implementação dos _VBOs_ provou ser uma melhoria significativa na performance do programa, especialmente em modelos mais complexos.

De facto, à medida que as fases vão avançado, o grupo tem vindo a adquirir imenso conhecimento sobre este mundo da computação gráfica e está ansioso para a fase final.

#pagebreak()

#bibliography("bibliography.bib")
