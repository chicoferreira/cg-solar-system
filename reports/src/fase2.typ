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
void Engine::renderGroup(WorldGroup &group) {
    glPushMatrix()

    glMultMatrixf(*group.transform.mat);

    for (size_t model_index : group.models) {
        Model &model = m_models[model_index];
        renderModel(m_models[model_index]);
    }
    for (WorldGroup &child : group.children) {
        renderGroup(child);
    }

    glPopMatrix();
}
```)

Devido à função `glMultMatrixf` do _OpenGL_ interpretar as matrizes em _column-order_ em vez de _row-order_ (como é em _arrays_ bidimensionais em _C++_), a matriz final de transformação (@transform_equation) também tem que ser transposta antes de ser enviada para o _OpenGL_.

== Visualização e manipulação das transformações <transform_imgui>

#figure(image("fase2/transform_view.png", width: 91%), caption: [Visão geral das transformações no _ImGui_])

O grupo tem a ambição de tornar esta _engine_ *usável*. Isto implica que interações com o utilizador são essenciais e críticas. Com isto, recorrendo ao _ImGui_ @imgui, tornamos possível a visualização e edição das transformações que são carregadas para o mundo.

#figure(image("fase2/group_transform_view.png"), caption: [Lista de transformações num grupo no _ImGui_])

#let imgui_button(content) = box(stroke: black, radius: 5pt, inset: (x: 4pt), outset: (y: 2.4pt), content)

Um grupo tem uma lista de transformações. Uma transformação pode ser *removida* clicando no botão #imgui_button[Remove] presente à frente do nome dessa. Qualquer parâmetro de qualquer transformação pode ser alterado em tempo real, com feedback instantâneo no mundo.

É possível adicionar uma nova transformação ao grupo. Para isso, basta clicar no botão #imgui_button[Add Transformation] e escolher o tipo de transformação que se quer adicionar. A transformação é adicionada ao fim da lista de transformações do grupo.

#figure(image("fase2/add_transform.png"), caption: [Adição de uma nova transformação no _ImGui_])

Também é possível, arrastando uma transformação com o rato, trocar duas transformações de posições na lista entre elas.

#figure(grid(columns: 2, 
  image("fase2/drag_and_drop_pre.png"),
  image("fase2/drag_and_drop_pos.png")
  ), caption: [_Drag and Drop_ de transformações])

Com a mesma ação, dá também para mover a transformação de um grupo para outro.

#figure(grid(columns: 2, 
  image("fase2/drag_and_drop_wgroup_pre.png"),
  image("fase2/drag_and_drop_wgroup_pos.png")
  ), caption: [_Drag and Drop_ de transformações entre grupos])

Estas funcionalidades interativas serão essenciais para atingir o objetivo pretendido.

= Camera primeira pessoa

Como os requisitos desta fase não foram muito extensos, nem envolveram muitos _rewrites_ de código na _engine_, implementamos também o modo de camera em primeira pessoa.

Este modo baseia-se na mesma função `gluLookAt` que a camera em terceira pessoa também usa. Com isto, a alteração entre os modos de primeira-pessoa e terceira-pessoa não causam alterações na posição camera #footnote[Isto poderia acontecer devido a, por exemplo, usar-se variáveis de `yaw` e `pitch` diferentes para cada um dos modos.], o que torna a experiência de utilização muito mais intuitiva.

O cálculo de atualização da camera em primeira-pessoa conforme o movimento do rato é muito semelhante à atualização dela em terceira-pessoa. Como foi descrito no relatório anterior, a partir de cálculos de coordenadas esféricas a partir de cartesianas é possível conseguir os valores de raio e ângulos $alpha$ e $beta$ da camera atual, somar a diferença de movimento do rato a esses ângulos, e converter de volta para coordenadas cartesianas. A diferença é que, em vez de se atualizar as coordenadas de posição da camera, em primeira-pessoa, atualiza-se as coordenadas do _looking at_.

```cpp
void UpdateCameraRotation(world::Camera &camera, float x_offset, float y_offset) {
    x_offset = degrees_to_radians(x_offset) * sensitivity;
    y_offset = degrees_to_radians(y_offset) * sensitivity;

    float radius, alpha, beta;
    Vec3f direction = camera.position - camera.looking_at;
    if (camera.first_person_mode) {
        direction = -direction;
        y_offset = -y_offset;
    }

    direction.ToSpherical(radius, alpha, beta);

    alpha -= x_offset;
    beta -= y_offset;

    // Clamp -180deg<beta<180deg

    const Vec3f new_direction = Vec3fSpherical(radius, alpha, beta);
    if (camera.first_person_mode)
        camera.looking_at = new_direction + camera.position;
    else
        camera.position = new_direction + camera.looking_at;
}
```

Em primeira-pessoa, o vetor direção é invertido visto que, em primeira pessoa, queremos mover a camera à volta dela e não em torno do _looking at_. O _y offset_ também é invertido para o movimento do rato ser traduzido num movimento mais natural da camera.

No fim, a camera é atualizada com a soma do vetor direção com a posição da camera (em primeira-pessoa) ou com o _looking at_ (em terceira-pessoa), para que o movimento da camera seja feito em relação a esses pontos.

== Movimento

Para o movimento, a cada renderização de _frame_, a camera é alimentada com os valores de _input_ do teclado, se estiver em primeira pessoa, e _scroll_ do rato.

Este _input_ é um vetor que representa para onde o utilizador pretende andar.
- `W` pressionado $->$ $(0,0,1)$ $->$ Andar para frente
- `S` pressionado $->$ $(0,0,-1)$ $->$ Andar para trás
- `D` pressionado $->$ $(1,0,0)$ $->$ Andar para a direita
- `A` pressionado $->$ $(-1,0,0)$ $->$ Andar para a esquerda
- `Espaço` pressionado $->$ $(0,1,0)$ $->$ Subir
- `Control Esquerdo` pressionado $->$ $(0,-1,0)$ $->$ Descer

Para movimentações diagonais, por exemplo, várias destas teclas podem ser pressionadas ao mesmo tempo, e com isso, o vetor _input_ será a soma dos vetores associados a todas as teclas pressionadas.

Com o vetor _input_, são calculados dois vetores, um que representa a direção para a frente da camera, e outro que representa a direção para a direita da camera.

#math.equation(block: true, numbering: "(1)", $
arrow(f) = norm(arrow(italic("looking_at")) - arrow(italic("position")))
$)

#math.equation(block: true, numbering: "(1)", $
arrow(r) = norm(arrow(f) times arrow(italic("up")))
$)

Sendo, $arrow(f)$ o vetor de direção para a frente da camera, $arrow(r)$ o vetor de direção para a direita da camera, $arrow(italic("looking_at"))$, $arrow(italic("position"))$ e $arrow(italic("up"))$ vetores presentes nas informações da camera, $arrow(a) times arrow(b)$ o produto externo entre $arrow(a)$ e $arrow(b)$ e $norm(arrow(a))$ o vetor normalizado de $arrow(a)$.

Com isto, o vetor direção final pode ser calculado da seguinte forma:

#math.equation(block: true, numbering: "(1)", $
arrow(d) = norm(arrow(f) times arrow(i)_z + arrow(r) times arrow(i)_x) + arrow(italic("up")) times arrow(i)_y
$)

Sendo $arrow(i)$ o tal vetor de _input_ e as váriaveis denotadas anteriormente. Atenção que as multiplicações aqui (ex: $arrow(f) times arrow(i)_z$) não denotam produtos externos mas sim multiplicações de vetores por valores.

De notar que os movimentos no eixo $x z$ são normalizados, pois, caso contrário, o movimento na diagonal seria mais rápido do que o movimento horizontal #footnote[O vetor $(1,0,1)$ tem tamanho $sqrt(2)$, não é unitário.]. A normalização não inclui o eixo $y$ para dar mais liberdade de movimentação ao utilizador na subida e descida enquanto se move horizontalmente.

=== Suavidade no movimento

Para dar um _feedback_ de utilização agradável ao utilizador, a camera move-se conforme equações físicas de aceleração e velocidade. 

A cada _frame_, a camera é atualizada com a função `TickCamera` que recebe o vetor _input_ e o _timestep_ (tempo que passou desde o último _frame_). Este _timestep_ é calculado a partir da diferença do `glfwGetTime` @glfw entre o _frame_ atual e o _frame_ anterior #footnote[Isto trata-se apenas de uma aproximação visto que o _Framerate_ pode não ser constante, mas como não estamos num cenário onde operações têm de ser precisas (por exemplo, em colisões de objetos), não há grande problema.].

Desta forma, o nosso _loop_ principal da _engine_ foi alterado para incluir a atualização da camera a cada _frame_.

#align(center, ```cpp
void Engine::Run() {
    float currentTime = glfwGetTime();
    while (!glfwWindowShouldClose(m_window)) {
        ...
        const float newTime = glfwGetTime();
        ProcessInput(newTime - currentTime);
        currentTime = newTime;

        Render();
        ...
    }
}
```)

Para se concretizar o movimento suave, foram adicionados parâmetros de velocidade, velocidade máxima, aceleração e fricção à camera. Voltando à função `TickCamera`, esta função é responsável por atualizar esses parametros conforme o _input_ do utilizador.

Esses parametros são atualizados da seguinte forma:

#math.equation(block: true, numbering: "(1)", $
arrow(v) = arrow(d) times italic("camera")_italic("acceleration/s") times Delta t
$)

#math.equation(block: true, numbering: "(1)", $
arrow(p) = arrow(v) times Delta t
$)

Sendo $arrow(v)$ o vetor velocidade da camera, $arrow(d)$ o vetor direção calculado anteriormente, $italic("camera")_italic("acceleration/s")$ uma constante da camera, $Delta t$ o intervalo de tempo entre _frames_.

Caso o utilizador não esteja a pressionar nenhuma tecla de movimento de _input_, a velocidade da camera é atualizada da seguinte forma:

#math.equation(block: true, numbering: "(1)", $
arrow(v)_n = arrow(v)_a times italic("camera")_italic("friction/s") times Delta t
$)

Sendo $arrow(v)_n$ a nova velocidade, $arrow(v)_a$ a velocidade anterior e $italic("camera")_italic("friction/s")$ uma constante na camera.

A lógica de implementação desta funcionalidade pode ser vista aqui:

#[
#set text(size: 0.82em)
#set align(center)
```cpp
void TickCamera(world::Camera &camera, const Vec3f input, const float timestep) {
    const Vec3f forward = (camera.looking_at - camera.position).Normalize();
    const Vec3f right = forward.Cross(camera.up).Normalize();

    const Vec3f move_dir = (forward * input.z + right * input.x).Normalize() 
        + camera.up * input.y;
    const Vec3f acceleration = move_dir * camera.acceleration_per_sec * timestep;
    camera.speed += acceleration;

    if (camera.speed.Length() > camera.max_speed_per_second) {
        camera.speed = camera.speed.Normalize() * camera.max_speed_per_sec;
    }

    camera.position += camera.speed * timestep;
    camera.looking_at += camera.speed * timestep;

    if (move_dir.x == 0 && move_dir.y == 0 && move_dir.z == 0) {
        camera.speed -= camera.speed * timestep * camera.friction_per_second;
    }

    // Atualizar scroll
}
```
]

A mesma lógica também foi aplicada ao _scroll_ do rato que agora dá _zoom in_ e _zoom out_ suavemente independente do modo da camera (primeira ou terceira pessoa).

Estes parametros podem ser vistos e editados em tempo real na aba _"Camera"_ do _ImGui_.

#figure(image("fase2/camera.png", width: 75%), caption: [Visualização dos novos parâmetros da camera no _ImGui_])

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

== Modelos no _ImGui_

A visualização dos modelos no _ImGui_ também foi atualizada para constatar as mudanças nas estruturas de dados e mostrar novas informações sobre o modelo.

#figure(image("fase2/models.png", width: 70%), caption: "Visualização de modelos no ImGui")

= Serialização do Mundo

Para implementação da geração do mundo do sistema solar, será necessário uma forma programática de criação de mundos no formato _XML_. Para aproveitar as APIs já existentes de cálculos matemáticos, o carregamento do mundo da nossa _engine_ foi estendido para também suportar escrita de mundos em _XML_. 

Desta forma, agora, um mundo carregado em memória pode ser guardado num ficheiro _XML_, e um mundo guardado num ficheiro _XML_ pode ser carregado em memória, tendo assim operações de _serialização_ e _deserialização_ implementadas.

Esta funcionalidade foi implementada também com a biblioteca _tinyxml2_ @tinyxml2, que também já foi usada na fase anterior para a leitura de mundos.

Isto veio com uma vantagem que, o mundo pode ser guardado em _runtime_, estendendo assim a criação de mundos para que o utilizador, dinamicamente, possa modificar o mundo a partir das funcionalidades mostradas no capítulo @transform_imgui e guardá-lo em disco para uso posterior.

== Funcionalidade de _Reload_

#let reload_button = imgui_button[Reload]

Com o _refactor_ feito devido à implementação da serialização de mundos, também foi implementado um botão de #reload_button no _ImGui_, que relê o arquivo XML do mundo e altera o estado do mundo para o que foi lido. Este botão também funciona como um _reset_ da _scene_ caso não tenham havido mudanças.

#figure(image("fase2/reload.png", width: 70%), caption: [Botão de #reload_button no _ImGui_])


= Primeira versão do Sistema Solar

#heading(numbering:none)[Conclusão]

O projeto ta top. a visão que temos nele é se tornar um editor capaz de edição em tempo real de cenas complexas e elaboradas. O CI que foi feito na fase anterior já veio a ser útil a detetar algumas incompatibilidades ou mudanças no C++ do Linux/Mac em relação ao Windows.


#bibliography("bibliography.bib")
