#import "template.typ": *

#show: project.with(
  title: "Computação Gráfica - Fase 2",
  date: "3 de Abril de 2024",
  group: [*Grupo 2*],
  authors: (
    (name: "Daniel Pereira", number: "A100545"),
    (name: "Duarte Ribeiro", number: "A100764"),
    (name: "Francisco Ferreira", number: "A100660"),
    (name: "Rui Lopes", number: "A100643"),
  ),
)

#heading(numbering: none)[Introdução]

Este relatório descreve a segunda fase do projeto de Computação Gráfica. Nesta fase foi pedido uma extensão da fase anterior, onde foi pedido para ser implementado um sistema de hierarquias de transformações geométricas (translação, rotação e escala) onde são aplicadas aos modelos desse grupo e aos seus subgrupos. Para além disso, também foi pedido uma _demo_ de um sistema solar com a utilização do sistema de hierarquias desenvolvido.

= Hierarquia de Transformações

Para a implementação do sistema de hierarquias de transformações geométricas, foi criada uma estrutura de dados `GroupTransform` que cada `WorldGroup` #footnote[De relembrar que `WorldGroup` é a estrutura de dados que contém dados do grupo do mundo (lista de modelos, subgrupos e agora lista de transformações).] tem.

Este `GroupTransform` é essencialmente um _wrapper_ para uma matriz de transformação (uma matriz 4x4) com uma lista de transformações. Uma transformação pode ser:

- um `Translate(Vec3f translation)`: que representa uma translação sobre o vetor `translation`;
- um `Rotate(float angle, Vec3f axis)`: que representa uma rotação de `angle` graus sobre o eixo `axis`;
- um `Scale(Vec3f scale)`: que representa uma escala sobre o vetor `scale`.

#v(5pt)

A partir da lista de transformações, a matriz de transformação é calculada e armazenada no `GroupTransform`. Esta operação é feita no carregamento do mundo ou a cada vez que a lista de transformações é alterada.

As formulas para calcular cada tipo de transformação são as seguintes:

- Translação em $(x,y,z)$:
$
cal(T)(x,y,z) = mat(
    1, 0, 0, x;
    0, 1, 0, y;
    0, 0, 1, z;
    0, 0, 0, 1
)
$

- Escala em $(x,y,z)$:

$
cal(S)(x,y,z) = mat(
    x, 0, 0, 0;
    0, y, 0, 0;
    0, 0, z, 0;
    0, 0, 0, 1
)
$

- Rotação em torno de um eixo arbitrário $(x,y,z)$ por um ângulo $alpha$:

$
cal(R)(alpha,x,y,z) = mat(
    x^2 + (1 - x^2) cos(alpha), x y (1 - cos(alpha)) - z sin(alpha), x z (1 - cos(alpha)) + y sin(alpha), 0;
    x y (1 - cos(alpha)) + z sin(alpha), y^2 + (1 - y^2) cos(alpha), y z (1 - cos(alpha)) - x sin(alpha), 0;
    x z (1 - cos(alpha)) - y sin(alpha), y z (1 - cos(alpha)) + x sin(alpha), z^2 + (1 - z^2) cos(alpha), 0;
    0, 0, 0, 1
)
$


Com isto, a matriz de transformação de um `GroupTransform` é calculada multiplicando todas as matrizes de transformação na ordem em que foram adicionadas à lista.

A rotação sendo em torno de um eixo arbitrário, em oposição em torno de um eixo principal, dá mais flexibilidade ao artista

Por exemplo, para este conjunto de transformações no xml:

#align(center, ```xml
<group>
    <translate X="1" Y="0" Z="0"/>
    <rotate angle="90" X="0" Y="1" Z="0"/>
    <scale X="2" Y="1" Z="1"/>
</group>
```)

A matriz final de transformação seria ($I$ sendo a matriz identidade):

#math.equation(block: true, numbering: "(1)",
    $ R = I times cal(T)(1,0,0) times cal(R)(90deg,0,1,0) times cal(S)(2,1,1) $
) <transform_equation>

O código de operações sobre matrizes já tinha sido desenvolvido na fase anterior, logo esta funcionalidade foi desenvolvida sem muitas alterações.

== No _OpenGL_

Com a matriz de transformação pronta, foi apenas necessário chamar o método `glMultMatrixf` do _OpenGL_ @opengl com a matriz de transformação antes de renderizar os modelos do grupo. Esta função multiplica a matriz atual pela matriz selecionada atualmente (neste caso a `GL_MODELVIEW`), substituindo-a pela matriz resultante.

Devido a esta multiplicação, a função `glPushMatrix` e `glPopMatrix` foram usadas para guardar e restaurar a matriz anterior, respetivamente. Isto permite que a matriz atual seja restaurada para o estado anterior após a renderização do grupo.

#align(center, ```cpp
void renderGroup(WorldGroup &group) {
    glPushMatrix()

    glMultMatrixf(*group.transform.mat);

    for (auto &model : group.models) {
        renderModel(model);
    }
    for (auto &child : group.children) {
        renderGroup(child);
    }

    glPopMatrix();
}
```)

Devido à função `glMultMatrixf` do _OpenGL_ interpretar as matrizes em _column-order_ em vez de _row-order_ (como é em _arrays_ bidimensionais em _C++_), a matriz final de transformação (@transform_equation) também tem que ser transposta antes de ser enviada para o _OpenGL_.

== Visualização e manipulação das transformações <transform_imgui>

Mostrar IMGUI e as suas manipulação de transformações

Drag and Drop

= Camera primeira pessoa

== Movimento suave

Decoupled with framerate

= Repetição de Modelos

Devido à grande quantidade de luas e planetas no sistema solar, que irão utilizar por baixo o mesmo modelo de esfera, foi feita algumas alterações nas estruturas de dados para não haver duplicação de memória em modelos iguais #footnote[De relembrar que um modelo, atualmente, é simplesmente a lista de vértices que o representa graficamente.].

Anteriormente, a estrutura do modelo, estava guardada dentro da lista de modelos de um `WorldGroup`. Isto trará problemas de memória e lentidão de carregamento em cenas pesadas, que contenham modelos repetidos.

Agora, as estruturas dos modelos foi movida para dentro da estrutura do mundo completo, e dentro do `WorldGroup` guarda-se apenas um inteiro que representa o índice do modelo na lista de modelos do mundo.

#import "@preview/fletcher:0.4.3" as fletcher: diagram, node, edge

#import fletcher.shapes: diamond

#grid(columns: (1fr, 1fr), align: center + bottom,
  figure(block(align(center + horizon, {
    set text(size: 0.65em)
    diagram(
      node-stroke: 1pt,
      edge-stroke: 1pt,
            node((1,-1), [Mundo], corner-radius: 2pt),
      edge("-|>"),
      node((1,0), [Grupo Principal], corner-radius: 2pt),
      edge("-|>"),
      node((0,1), [Grupo 1], corner-radius: 2pt),

      let model(coords) = (
          edge("-|>"),
          node(coords, [
          sphere.3d

          #set text(size: 0.9em)
          Vertex:
          - (1,1,1)
          - (2,2,2)
          - ...
          ], corner-radius: 2pt)
      ),

      model((0,2)),

      node((1,1), [Grupo 2], corner-radius: 2pt),
      model((1,2)),
      
      node((2,1), [Grupo 3], corner-radius: 2pt),
      model((2,2)),

      edge((1,0), (0,1), "-|>"),
      edge((1,0), (1,1), "-|>"),
      edge((1,0), (2,1), "-|>"),
    )
  })), caption: [Sem identificadores de modelo]),
  figure(block(height: 25%, align(center + horizon, {
    set text(size: 0.65em)
    diagram(
      node-stroke: 1pt,
      edge-stroke: 1pt,

      node((1,-1), [Mundo], corner-radius: 2pt),
      edge("-|>"),
      node((1,0), [Grupo Principal], corner-radius: 2pt),
      edge("-|>"),
      node((0,1), [Grupo 1], corner-radius: 2pt),

      let model(coords) = (
          edge("-|>"),
          node(coords, [
          Model Id \#1
          ], corner-radius: 2pt)
      ),

      model((0,2)),

      node((1,1), [Grupo 2], corner-radius: 2pt),
      model((1,2)),
      
      node((2,1), [Grupo 3], corner-radius: 2pt),
      model((2,2)),

      edge((1,0), (0,1), "-|>"),
      edge((1,0), (1,1), "-|>"),
      edge((1,0), (2,1), "-|>"),

      node((2,-0.7), [
          *Model Id \#1* \
          sphere.3d

          #set text(size: 0.9em)
          Vertex:
          - (1,1,1)
          - (2,2,2)
          - (3,3,3)
          - ...
          ], corner-radius: 2pt, shape: rect),
    
      edge((1,-1), (2,-1), "-|>")
    )
  })), caption: [Com identificadores de modelo])
)



= Serialização do Mundo

Para implementação da geração do mundo do sistema solar, será necessário uma forma programática de criação de mundos no formato _XML_. Para aproveitar as APIs já existentes de cálculos matemáticos, o carregamento do mundo da nossa _engine_ foi estendido para também suportar escrita de mundos em _XML_. 

Desta forma, agora, um mundo carregado em memória pode ser guardado num ficheiro _XML_, e um mundo guardado num ficheiro _XML_ pode ser carregado em memória, tendo assim operações de _serialização_ e _deserialização_ implementadas.

Esta funcionalidade foi implementada também com a biblioteca _tinyxml2_ @tinyxml2, que também já foi usada na fase anterior para a leitura de mundos.

Isto veio com uma vantagem que, o mundo pode ser guardado em _runtime_, estendendo assim a criação de mundos para que o utilizador, dinamicamente, possa modificar o mundo a partir das funcionalidades mostradas no capítulo @transform_imgui e guardá-lo em disco para uso posterior.

= Primeira versão do Sistema Solar

#heading(numbering:none)[Conclusão]

O projeto ta top. a visão que temos nele é se tornar um editor capaz de edição em tempo real de cenas complexas e elaboradas. O CI que foi feito na fase anterior já veio a ser útil a detetar algumas incompatibilidades ou mudanças no C++ do Linux/Mac em relação ao Windows.


#bibliography("bibliography.bib")
