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

#text(fill:red)[A FAZER]

Adicionado parametro de nome nos grupos

= Simulação de tempo

Com grande foco desta fase, uma componente temporal teve que ser adicionada ao projeto. Esta é responsável por simular o tempo passado, permitindo a implementação das várias funcionalidades temporais que foram pedidas.

Como já tinhamos implementado o processamento do tempo na fase anterior para as acelerações e desacelerações da camera, foi relativamente simples adicionar a componente temporal ao projeto.

Agora, a `Engine` guarda também o tempo atual, que é atualizado a cada _frame_. A cada _frame_ é somado ao tempo atual o tempo que passou desde o último _frame_, permitindo assim a simulação do tempo. Com esta abordagem também é possível controlar a velocidade do tempo através de um fator de escala e também pausá-lo.

#figure(```cpp
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

#text(fill:red)[IMAGEM DO IMGUI DA ABA DE SIMULAÇÃO]

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

De regra geral deste projeto, esta transformação também foi adicionada no _ImGui_, onde é possível controlar o tempo que demora a completar uma rotação completa e o eixo de rotação.

#text(fill:red)[IMAGEM DO IMGUI DA ABA DE ROTAÇÃO TEMPORAL]

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

#text(fill:red)[IMAGEM DO CAMINHO DA CURVA DE CATMULL ROM]

O caminho é formado por pontos #footnote[Para já são 100 pontos, um valor não configurável.] ligados entre si, espaçados uniformemente ao longo da curva. Estes pontos são enviados para o _OpenGL_ e são renderizados como `GL_LINE_LOOP`. A forma de como são enviados para o _OpenGL_ será explicada num #link(<path_vbo>)[capítulo posterior].

=== Integração no ImGui

É possível adicionar e remover pontos na execução do programa.

#text(fill:red)[MOSTRAR ADICIONAR PONTO, REMOVER PONTO, ALTERAR PONTO, MENOS QUE 4 PONTOS = ERRO, MOSTRAR CAMINHO LIGADO LOCALMENTE E GLOBALMENTE]

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

#text(fill:red)[INSERIR PRINT DO IMGUI DOS MODELOS]

== Diferenças de performance (_Benchmarks_)

Para comparar diferenças da implementação de renderização imediata em comparação com a renderização com _VBOs_ com índices, foi escolhido o sistema solar da fase anterior. Para referência esta cena tem X triângulos com X índices.

Não foi possível escolher uma cena mais recente, visto que os modelos sofreram alterações nos índices, pelo que, por exemplo, não temos desenvolvido geração de modelos sem índices para _patches_ de _Bezier_.

#table(columns: 3, align: center + horizon, 
  [Sistema solar], [_Frametime_ (ms)], [FPS],
  [Sem _VBOs_], [], [],
  [Com _VBOs_ com índices], [], []
)

#text(fill:red)[COMPLETAR BENCHMARK]

= Geração de Bezier Patches

= Sistema Solar com rotações temporais e asteroides

== Adição do tempo 

== Asteroides

== Cometa


#heading(numbering: none)[Conclusão]

#text(fill:red)[A FAZER]

#pagebreak()

#bibliography("bibliography.bib")