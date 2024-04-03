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

Este relatório descreve a segunda fase do projeto de Computação Gráfica. Nesta fase foi pedido uma extensão da fase anterior, onde foi pedido para ser implementado um sistema de hierarquias de transformações geométricas (translação, rotação e escala) onde são aplicadas aos modelos desse grupo e aos seus subgrupos. Para além disso, também foi pedido uma _demo_ do sistema solar com a utilização do sistema de hierarquias desenvolvido.

= Hierarquia de Transformações

Para a implementação do sistema de hierarquias de transformações geométricas, foi criada uma estrutura de dados `GroupTransform` que cada `WorldGroup` #footnote[De relembrar que `WorldGroup` é a estrutura de dados que contém dados do grupo do mundo (lista de modelos, subgrupos e agora lista de transformações).] tem.

Este `GroupTransform` é essencialmente um wrapper para uma matriz de transformação (uma matriz 4x4) com uma lista de transformações. Uma transformação pode ser:

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

A matriz de final transformação seria ($I$ sendo a matriz identidade):

$ I times cal(T)(1,0,0) times cal(R)(90deg,0,1,0) times cal(S)(2,1,1) $

O código de operações sobre matrizes já tinha sido desenvolvido na fase anterior.

== OpenGL

Com a matriz de transformação pronta, foi apenas necessário chamar o método `glMultMatrixf` do OpenGL @opengl com a matriz de transformação antes de renderizar os modelos do grupo. Esta função multiplica a matriz atual pela matriz selecionada atualmente (neste caso a `GL_MODELVIEW`), substituindo-a pela matriz resultante.

Devido a esta multiplicação, a função `glPushMatrix` e `glPopMatrix` foram usadas para guardar e restaurar a matriz anterior, respetivamente. Isto permite que a matriz atual seja restaurada para o estado anterior após a renderização do grupo.

#align(center, ```cpp
void renderGroup(WorldGroup &group) {
    glPushMatrix();

    const auto mat = group.transform.mat;
    glMultMatrixf(*mat);

    for (auto &model : group.models) {
        renderModel(model);
    }

    for (auto &child : group.children) {
        renderGroup(child);
    }

    glPopMatrix();
}
```)



== Visualização e manipulação das transformações

Mostrar IMGUI e as suas manipulação de transformações

Drag and Drop

= Camera primeira pessoa

== Movimento suave

Decoupled with framerate

= Repetição de Modelos

falar tambem da Nova view dos modelos https://github.com/chicoferreira/cg-solar-system/issues/29.

= Serialização do Mundo

= Primeira versão do Sistema Solar

#heading(numbering:none)[Conclusão]

O projeto ta top. a visão que temos nele é se tornar um editor capaz de edição em tempo real de cenas complexas e elaboradas. O CI que foi feito na fase anterior já veio a ser útil a detetar algumas incompatibilidades ou mudanças no C++ do Linux/Mac em relação ao Windows.


#bibliography("bibliography.bib")