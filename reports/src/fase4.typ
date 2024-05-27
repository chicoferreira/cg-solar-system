#import "template.typ": *

#show: project.with(
  title: "Computação Gráfica - Fase 4",
  date: "26 de Maio de 2024",
  group: [*Grupo 2*],
  authors: (
    (name: "Daniel Pereira", number: "A100545"),
    (name: "Duarte Ribeiro", number: "A100764"),
    (name: "Francisco Ferreira", number: "A100660"),
    (name: "Rui Lopes", number: "A100643"),
  ),
)

#heading(numbering: none)[Introdução]

Este relatório tem como objetivo apresentar a quarta e última fase do projeto de Computação Gráfica. Nesta fase, foram implementados luzes, texturas e _frustum culling_. Além disso, foram realizadas algumas melhorias no _ImGui_.

= Luzes

Como grande objetivo desta fase, foram implementadas luzes na engine. Como pedido, foram implementadas 3 tipos de luzes: luz pontual, direcional e de foco. As luzes foram feitas usando a implementação padrão de luzes do OpenGL. 

== Normais de Modelos

Para a implementação das luzes, foi necessário calcular as normais dos modelos. Para tal, o programa gerador foi alterado para também calcular as normais dos modelos que ele gera. As normais de um modelo são carregadas para a GPU no início do programa para um _buffer_ próprio.

=== Normais do Plano

O plano acabou por ser o modelo mais simples de se calcular as normais. Como é um plano, as normais são todas iguais e apontam todas para cima. Portanto, para cada vértice do plano, a normal é (0, 1, 0).

=== Normais da Esfera

Para a esfera também acaba por ser simples. Para cada vértice da esfera, a normal é o vetor que liga o centro da esfera ao vértice. Como a normal deve ser um vetor unitário, calcula-se o vetor com coordenadas esféricas com os mesmos parâmetros do vértice, mas com raio 1.

=== Normais do Cubo

Como a nossa implementação do nosso cubo é feita a partir da geração do plano, em que para cada face, é aplicado uma matriz que coloca o plano na posição da face correta, bastou-se aplicar a mesma transformação às normais que já tínhamos calculado para o plano. Isto surgiu ser um problema devido a também termos translações do plano nessa mesma transformação. Ora, obviamente não queremos que a translação do plano seja adicionada à normal. Desta forma teve de se separar a transformação em duas, uma só com rotações e uma só com translações, e só aplicar as transformações de rotação no cálculo das normais.

=== Normais do Cilindro

As normais do cilindro já são um pouco mais complicadas, devido a haver três partes diferentes do cilindro: laterais, base superior e base inferior. Para as laterais, igualmente como na esfera, a normal é o vetor que liga o centro do cilindro com altura do vértice ao vértice, mas com raio 1. Para as bases, a normal para todos os vértices é simplesmente o vetor (0, 1, 0) para a base superior e (0, -1, 0) para a base inferior.

Devido à base ter uma descontinuidade em relação às laterais, apesar das coordenadas dos pontos que os unem serem o mesmo, dois vértices tiveram de ser colocados na mesma posição, um com a normal da base e outro com a normal das laterais. Isto desfaz a otimização que foi feita nas fases anteriores de não repetir vértices entre os vértices que unem as laterais com as bases. #footnote[O mesmo acontecerá com a otimização de não repetir vértices entre o fim da volta completa das laterais e o início da volta seguinte, quando forem implementadas texturas.]

=== Normais do Cone

O cone, em semelhança ao cilindro, também é composto por mais de uma parte: a base e a lateral. A base do cone é igual à base do cilindro, para cada vértice da base, a sua normal é (0, -1, 0). Para as laterais já é feito alguns cálculos. Inicialmente é calculado o ângulo que a superfície lateral faz com um vetor que vai do topo do cone até à base com $alpha = tan^(-1)(r/h)$. Por semelhança de triângulos, este ângulo será o ângulo que a normal faz com a base. 

#figure(image("fase4/cone.png", width: 30%), caption: [Demonstração do cálculo da normal do cone #footnote[Retirado de: https://stackoverflow.com/questions/19245363/opengl-glut-surface-normals-of-cone.]])

Com isto, tendo o ângulo horizontal já calculado das outras fases podemos calcular as normais usando coordenadas esféricas com: $cal(S)(italic("circleAngle"), alpha, 1)$, sendo $cal(S)(alpha, beta, r)$ a função que devolve a coordenada cartesiana a partir de coordenadas esféricas $(alpha,beta,r)$.

Em semelhança ao cilindro, a otimização de juntar vértices da base com a lateral também teve de ser retirada. Para além disso, a otimização de reusar o vértice do topo para os vários triângulos juntos a ele, como agora terão normais diferentes, também teve de ser removida. 

=== Normais das _Bezier Patches_

Utilizando os cálculos da fase anterior para calcular os vértices dos _patches_, podemos facilmente calcular as derivadas parciais verticais, $v$, e horizontais, $u$, com #footnote[É recomendado a leitura do capítulo 4.2 do relatório da fase 3.]:
$
arrow(u) = (delta cal(P)(u,v))/(delta u) = mat(3u^2,2u,1,0) times M times P times M times mat(v^3;v^2;v;1)
$

$
arrow(v) = (delta cal(P)(u,v))/(delta v) = mat(u^3,u^2,u,1) times M times P times M times mat(3v^2;2v;1;0)
$

Com isto, a normal é simplesmente calculada com um produto externo:

$
arrow(n) = arrow(v) times arrow(u)
$

Este cálculo é executado para cada vértice e com isto, conseguimos calcular as normais para todo o _patch_.

=== Visualização de normais

De forma a visualizarmos possíveis problemas com as normais, foi implementado uma visualização de normais para um modelo.

Para isto, aquando da renderização de um modelo, para cada vértice de modelo é feita uma linha #footnote[Recorrendo ao `GL_LINES` do _OpenGL_.] desde esse vértice até ao vértice do modelo somado com o vetor da normal desse vértice.

Como isto é um modo para ser usado para depuração, para manter a sua implementação simples, isto não é renderizado a partir de VBOs mas sim usando operações no modo imediato do _OpenGL_, pelo que, para cenas grandes, performance pode ser significativamente afetada.

#figure(image("fase4/mostrar normais.png", width: 50%), caption: [Visualização de normais de um modelo])

Estas normais serão apenas uma aproximação visto que, caso o modelo seja escalado, a representação gráfica das normais também será escalada da mesma forma, apesar de terem sempre tamanho unitário. Como este modo só é usado para depuração, este detalhe também foi ignorado.

Esta visualização está desligada por padrão e pode ser ligada nas _settings_ no _ImGui_.

== Modelo de luzes

De forma a visualizar posições das luzes e o seu tipo na cena atual, renderizamos também um modelo representativo de uma luz na sua posição.

Para a luz pontual é simplesmente renderizado um círculo na sua posição.

#figure(image("fase4/point light model.png"), caption: [Representação gráfica de uma luz pontual])

Para a luz direcional é renderizado uma linha a partir da origem com a sua direção.

#figure(image("fase4/directional light model.png"), caption: [Representação gráfica de uma luz direcional])

Para a luz de foco é renderizado um ponto na sua posição (como na luz pontual) e uma linha que representa a sua direção.

#figure(image("fase4/spotlight light model.png"), caption: [Representação gráfica de uma luz de foco])

Esta renderização foi útil para resolvermos problemas em relação às luzes. Por ser uma opção de _debug_ não está ligada no início do programa, mas pode ser ligada a partir do menu de _settings_ no _ImGui_.

== Integração com o _ImGui_

Os parâmetros das luzes também podem ser configurados em tempo real a partir do _ImGui_ a partir de um menu próprio das luzes.

#figure(image("fase4/luzes imgui.png"), caption: [Menu de luzes no _ImGui_])

Também é possível remover e adicionar novas luzes.

#figure(image("fase4/luzes adicionar imgui.png"), caption: [Adicionar uma nova luz no _ImGui_])

Como estamos a usar a implementação padrão do _OpenGL_, e esta só suporta até 8 luzes em simultâneo, um aviso é colocado no menu quando o utilizador tem mais de 8 luzes adicionadas em simultâneo. O comportamento da engine neste caso é ignorar as luzes a partir da oitava.

#figure(image("fase4/luzes imgui oito.png"), caption: [Aviso de mais de 8 luzes no _ImGui_])

== Materiais

Como foram implementadas luzes, também teve de ser adicionado um material aos modelos de um grupo. O material é composto por 4 cores diferentes: ambiente, difusa, especular e emissiva, e um coeficiente de brilho. A implementação do material seguiu os requisitos do enunciado. A forma de como a representação dos modelos foi alterada e a sua integração no _ImGui_ será explicada num próximo capítulo.

= Texturas de Modelos

Outro grande requisito desta fase é adicionar a funcionalidade de modelos poderem ter uma textura. As texturas são carregadas no início do programa para um buffer de texturas do _OpenGL_ e são associadas a um modelo através de um índice.

== Coordenadas de Textura

Para começar a implementação de texturas, foi necessário especificar de como as texturas são aplicadas aos modelos. Para tal, foi necessário adicionar um novo vetor de coordenadas de textura a cada vértice de um modelo. Este vetor é um vetor de 2 dimensões que varia entre 0 e 1. As coordenadas de textura são enviadas para a GPU para um _buffer_ próprio no início do programa para cada modelo.

=== Plano

Para o plano, as coordenadas de textura são simples. Para cada vértice, a coordenada de textura é a iteração vertical e horizontal do vértice dividido pelo número de iterações.

Como o plano começa a ser gerado pelo topo esquerdo, as coordenadas de textura são geradas com o $z$ invertido ($z_n = 1.0 - z_i$) de forma que o topo esquerdo tenha coordenadas de textura (0, 1). 

=== Esfera

Para a esfera, as coordenadas de textura são semelhantes à do plano. Neste caso, a iteração atual do _stack_ e do _slice_ são divididos pelo número de _stacks_ e _slices_ respetivamente. Como começámos a gerar a esfera pela base, as coordenadas de textura não tiveram de ser invertidas como no plano.

Com as coordenadas de textura, muitas otimizações de agrupamento de vértices tiveram de ser retiradas. A otimização de juntar os vértices do fim da volta com o início da volta teve de ser retirado, devido a que, nessa junção tinha que ter duas coordenadas de textura diferentes, no fim $(1,y)$ e no início $(0,y)$. 

Da mesma forma, para os vértices da base e do topo, visto que os vários triângulos que uniam o topo/base ao resto da esfera têm coordenadas de textura diferentes, a otimização de os juntar também teve de ser removida.

=== Cubo

Para o cubo, como estamos a gerá-lo a partir de transformações do plano, as coordenadas de textura são iguais às do plano. No entanto teve de se ter atenção às transformações de rotação que estavam a ser aplicadas para que o cubo apresentasse as texturas com a orientação correta.

Não foi feita qualquer especialização de textura para o cubo. Para aproveitar a textura fornecida pelos docentes para o teste 4.6, a textura é repetida em todas as faces. 

=== Cilindro

Para o cilindro, as coordenadas de textura e o formato das texturas é igual ao que foi feito na ficha prática 11. A imagem da textura é dividida em 3 partes: o corpo a base e o topo.

O mapeamento da textura pode ser visto na imagem seguinte:

#figure(image("fase4/coordenadas textura cilindro.png", width: 40%), caption: [Mapeamento da textura do cilindro])

A sua explicação pode ser vista com grande detalhe nos slides fornecidos pelos docentes da ficha prática 11.

Também como a esfera, as otimizações de juntar vértices do fim da volta com o início da volta e do topo com os triângulos que o unem à lateral tiveram de ser retiradas, já que as coordenadas de textura são diferentes entre o início e o fim da volta das laterais.

=== Cone

Para o cone, as coordenadas de textura são semelhantes às da esfera, em que, para cada _stack_ e _slice_ a sua coordenada de textura é $(italic("slice") / italic("slices"), italic("stacks") / italic("stacks"))$. Isto irá fazer com que a textura fique mapeada à volta da lateral do cone. A otimização de usar os mesmos vértices do início da lateral com o fim da lateral também teve de ser retirada pelos mesmos motivos que foram citados para outros modelos.

Para a textura da base do cone, como não fizemos nenhuma especialização do formato da imagem desta textura, as coordenadas de textura representarão apenas um círculo centrada no centro da textura original, calculada a partir de: $(0.5, 0.5) + cal(P)(0.5, (italic("slice") times 2 pi)/"slices")$, com $cal(P)(r, alpha)$ sendo o cálculo da coordenada cartesiana a partir de uma coordenada polar.

=== _Bezier Patch_

Para os _patches_, as coordenadas de textura para cada par ($u$,$v$), como $u in [0,1] and v in [0,1]$, são exatamente $(1.0-u, 1.0-v)$. Isto faz com que a textura fique mapeada à volta do _patch_ corretamente com a orientação correta. A textura é repetida para cada _patch_.

== Formato final dos ficheiros .3d

Para acomodar as novas informações dos modelos (normais e coordenadas de texturas), o formato do ficheiro .3d foi alterado para guardar essas informações. Agora as normais e coordenadas de texturas são guardadas depois da informação dos vértices.

#figure(```
<número de vértices> <número de índices>
<x do vértice 1> <y do vértice 1> <z do vértice 1>
<x do vértice 2> <y do vértice 2> <z do vértice 2>
...

<x da normal 1> <y da normal 1> <z da normal 1>
<x da normal 2> <y da normal 2> <z da normal 2>
...

<x da coordenada de textura 1> <y da coordenada de textura 1>
<x da coordenada de textura 2> <y da coordenada de textura 2>
...

<x do índice 1> <y do índice 1> <z do índice 1>
...
```, caption: [Novo formato dos ficheiros .3d])

== Estrutura final de um modelo na engine

Para acomodar todas estas mudanças estruturais na _engine_, agora, cada grupo tem uma lista de modelos de um grupo. Esse modelo de um grupo guarda um identificador para o modelo real na _engine_ (que tem guardado o identificador do buffer da sua representação válida na GPU), o seu material com as 5 componentes, e um identificador para a textura real na _engine_. 

#import "@preview/fletcher:0.4.3" as fletcher: diagram, node, edge

#import fletcher.shapes: diamond

#figure(block(height: 31%, align(center + horizon, {
  set text(size: 0.65em)
  diagram(
    node-stroke: 1pt,
    edge-stroke: 1pt,

    node((1,-2), [Engine], corner-radius: 2pt),
    edge("-|>"),
    node((1,-1), [Mundo], corner-radius: 2pt),
    edge("-|>"),
    node((1,0), [Grupo Principal], corner-radius: 2pt),
    edge("-|>"),
    node((0,1), [Grupo 1], corner-radius: 2pt),

    let model(coords, info) = (
        edge("-|>"),
        node(coords, info, corner-radius: 2pt)
    ),

    model((0,2), [
        Model Id \#1\
        Material\
        Textura Id \#1
        ]),

    node((1,1), [Grupo 2], corner-radius: 2pt),
    model((1,2), [
        Model Id \#1\
        Material\
        Sem Textura
        ]),
    
    node((2,1), [Grupo 3], corner-radius: 2pt),
    model((2,2), [
        Model Id \#1\
        Material\
        Textura Id \#2
        ]),

    edge((1,0), (0,1), "-|>"),
    edge((1,0), (1,1), "-|>"),
    edge((1,0), (2,1), "-|>"),

    node((2,-1.7), [
        *Model Id \#1* \
        sphere.3d

        #set text(size: 0.9em)
        Lista de Vértices\
        Lista de Normais\
        Lista de Coordenadas de Textura
        ], corner-radius: 2pt, shape: rect),

    edge((1,-2), (2,-2), "-->"),

    node((0,-1.8), [
    *Textura Id \#1* \
    terra.png

    #set text(size: 0.9em)
    Lista de RGBA
    ], corner-radius: 2pt, shape: rect),

    edge((1,-2), (0,-2), "-->")
  )
  })), caption: [Estrutura final da representação do mundo na engine])

== Integração com o ImGui

Agora no _ImGui_ é possível visualizar e alterar as componentes do material bem como a textura, em tempo real.

Cada valor de RGBA da cor para cada material é apresentado, sendo possível alterá-lo arrastando o rato em cima dele. Ao passar o rato por cima do _preview_ da cor que é apresentada antes do nome do componente, é possível observar representações da cor em outros formatos.

#figure(image("fase4/model imgui.png", width: 90%), caption: [Menu de um modelo no _ImGui_])

Um _preview_ da imagem da textura atual é também é apresentada. Clicando no nome da textura, um _dropdown_ é aberto e é possível escolher qualquer outra textura que esteja carregada no programa.

#figure(image("fase4/model imgui textura.png", width: 90%), caption: [Menu de um modelo no _ImGui_ (Mudar textura)])

#block(breakable: false)[
Ao clicar na cor de cada um dos componentes é possível abrir um menu de _Color Picker_, onde a cor pode ser escolhida com mais detalhe.

#figure(image("fase4/model imgui material.png", width: 90%), caption: [Menu de um modelo no _ImGui_ (_Color Picker_)])
]

= Frustum Culling

Com vista a melhorar a performance do projeto, o grupo decidiu implementar _frustum culling_. Esta técnica consiste em verificar se um objeto está dentro do frustum da câmara antes de o enviar para a GPU. Se o objeto não estiver dentro do frustum, então não é enviado.

O principal problema a endereçar é, então, como verificar se um objeto está ou não dentro do frustum. Existem diversas formas de o fazer, que passam por criar um _bounding volume_ #footnote[https://en.wikipedia.org/wiki/Bounding_volume.] para cada objeto. Existindo diversos tipos de bounding volumes, a nossa escolha foram os _Axis-Aligned Bounding Boxes_ (_AABBs_) - essencialmente, um paralelepípedo alinhado com os principais eixos.

Esta funcionalidade pode ser ativada ou desativada a partir do menu de _settings_ do _ImGui_.

A implementação desta funcionalidade foi altamente baseada #link("https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling")[neste artigo do LearnOpenGL].

== Cálculo do AABB

Um AABB é definido por dois pontos, o seu canto inferior esquerdo e o seu canto superior direito. Assim, à medida que os vértices de um dado objeto vão sendo lidos, o seu AABB vai sendo atualizado. Se o vértice atualmente lido for o primeiro, então o AABB é inicializado com as coordenadas do vértice. Caso contrário, o AABB é atualizado de acordo com as coordenadas do vértice, se estas forem menores ou maiores que as coordenadas atuais do AABB.

É possível visualizar (e, de facto, executar) tal comportamento através do código da seguinte função:

#figure[```cpp
void AABB::Extend(Vec3f f)
{
    min.x = isnan(min.x) ? f.x : std::min(min.x, f.x);
    min.y = isnan(min.y) ? f.y : std::min(min.y, f.y);
    min.z = isnan(min.z) ? f.z : std::min(min.z, f.z);
    max.x = isnan(max.x) ? f.x : std::max(max.x, f.x);
    max.y = isnan(max.y) ? f.y : std::max(max.y, f.y);
    max.z = isnan(max.z) ? f.z : std::max(max.z, f.z);
}
```]

No entanto, esta simples abordagem tem um problema. Se o objeto tiver sofrido uma (ou várias) transformações, então o AABB, que tem coordenadas locais ao modelo, não estará correto. Ou seja, é necessário transformar o AABB de acordo com a matriz de transformação do objeto.

Tal pode ser efetuado aplicando a matriz de transformação a cada um dos vértices do AABB. No entanto, é preciso ter cuidado e manter as comparações entre os mínimos e os máximos, visto que com as transformações os extremos do AABB podem vir a ser alterados.

Em pseudocódigo, um algoritmo simples algo como:

#figure[```cpp
Vec3f corners[8];

// transform the first corner
Vec3f tmin = (transform * corners[0].ToVec4f(1.0f)).ToVec3f();
Vec3f tmax = tmin; 

// transform the remaining corners
for (int i = 1; i < 8; i++)
{
  Vec3f t = (transform * corners[i].ToVec4f(1.0f)).ToVec3f();
  tmin = min(tmin, t);
  tmax = max(tmax, t);
}

AABB rbox;

rbox.min = tmin;
rbox.max = tmax;

return rbox;
```]

Após alguma pesquisa, o grupo deparou-se com uma solução que utiliza, como base, a definição de um AABB através do seu centro e um vetor de extensão do centro até a um dos cantos. Esta foi a solução final pela qual optamos no nosso projeto, visto ser muito mais eficiente e simples de implementar e pode ser encontrada #link("https://gist.github.com/cmf028/81e8d3907035640ee0e3fdd69ada543f")[aqui].

#figure[```cpp
AABB AABB::Transform(Mat4f matrix)
{
  Vec3f center = (max + min) * 0.5;
  Vec3f extents = max - center;

  // transform center
  Vec3f t_center = (matrix * center.ToVec4f(1.0f)).ToVec3f();

  // transform extents (take maximum)
  Mat4f abs_mat = matrix.Abs();
  Vec3f t_extents = (abs_mat * extents.ToVec4f(0.0f)).ToVec3f();

  // transform to min/max box representation
  Vec3f tmin = t_center - t_extents;
  Vec3f tmax = t_center + t_extents;

  AABB rbox;

  rbox.min = tmin;
  rbox.max = tmax;

  return rbox;
}
```]

== Cálculo do Frustum

Uma peça fundamental do _frustum culling_ é, obviamente, o cálculo do _frustum_. O _frustum_ é definido por 6 planos, cada um representando um dos lados do _frustum_. Estes planos são definidos por um ponto e um vetor normal.

Definindo algumas normas para os cálculos que serão apresentados:
- $norm(arrow(a))$, o vetor $arrow(a)$ normalizado;
- $arrow(p)$, a posição da câmera;
- $arrow(l)$, o _looking at_ da câmera;
- $arrow(d) = norm(arrow(l) - arrow(p))$, a direção da câmera;
- $arrow("up")$, o vetor _up_ da câmera;
- $arrow(r) = norm(arrow(d) times arrow("up"))$, o vetor que aponta para a direita da câmera;
- $arrow(u) = norm(arrow(r) times arrow(d))$, o vetor que aponta para cima da câmera;
- $italic("VerticalH") = italic("far") times tan(italic("fov")/2)$, a metade do tamanho vertical do plano _far_;
- $italic("HorizontalH") = italic("VerticalH") times italic("aspect")$, a metade do tamanho horizontal do plano _far_;
- _near_, _far_, _aspect_ e _fov_, parâmetros da  câmera que o nome indica o seu propósito;
E ainda:
- $arrow(o)$, o vetor da posição do plano que vai ser calculado;
- $arrow(n)$, o vetor da normal do plano que vai ser calculado;

=== Plano _near_

O plano _near_ pode ser definido pelo ponto resultante da interseção do vetor direção da câmara com o plano _near_ e pelo próprio vetor direção da câmara. 

$
arrow(o) = arrow(p) + arrow(d) times italic("near") \
arrow(n) = arrow(d)
$

=== Plano _far_

Da mesma forma que o plano _near_, o plano _far_ pode ser definido pelo ponto resultante da interseção do vetor direção da câmara com o plano _far_ e pelo próprio vetor direção da câmara, mas com o sinal invertido, visto que o plano _far_ é oposto ao plano _near_.

$
arrow(o) = arrow(p) + arrow(d) times italic("far") \
arrow(n) = -arrow(d)
$

=== Plano _left_

O plano _left_ pode ser definido através do ponto de origem da câmara e um vetor $K$. Este vetor $K$ é calculado através do produto externo entre um vetor que une a origem da câmara ao ponto esquerdo central do plano _far_ e o vetor _up_ real da câmara. 


$
arrow(o) = arrow(p)\
arrow(n) = ((arrow(d) times italic("far")) - (arrow(r) times italic("HorizontalH"))) times arrow(u)
$

=== Plano _right_

O plano _right_ é definido de forma semelhante ao plano _left_, mas com o ponto central direito do plano _far_.

$
arrow(o) = arrow(p)\
arrow(n) = arrow(u) times ((arrow(d) times italic("far")) + (arrow(r) times italic("HorizontalH")))
$

=== Plano _top_

O plano _top_ é definido de forma semelhante, mas com o ponto central superior do plano _far_.

$
arrow(o) = arrow(p)\
arrow(n) = arrow(r) times ((arrow(d) times italic("far")) - (arrow(r) times italic("VerticalH")))
$

=== Plano _bottom_

O plano _bottom_ é definido de forma semelhante ao plano _top_, mas com o ponto central inferior do plano _far_.

$
arrow(o) = arrow(p)\
arrow(n) = ((arrow(d) times italic("far")) + (arrow(u) times italic("VerticalH"))) times arrow(r)
$

== Verificação de Interseção

Para verificar se um objeto deve ser enviado para a GPU renderizar, é necessário verificar se o seu AABB está dentro do _frustum_. Isto é, verificar, para cada um dos planos do frustum, se o AABB está do lado positivo desse plano.

Para isso é feito esse teste para cada plano com o código:
#figure[```cpp
bool Frustum::HasInside(AABB &aabb) const
{
    return (
      aabb.isOnOrForwardPlane(leftFace) 
      && aabb.isOnOrForwardPlane(rightFace) 
      && aabb.isOnOrForwardPlane(topFace) 
      && aabb.isOnOrForwardPlane(bottomFace) 
      && aabb.isOnOrForwardPlane(nearFace) 
      && aabb.isOnOrForwardPlane(farFace)
    );
}
```]

É de reparar que a ordem do teste dos planos foi escolhida cuidadosamente, em que os planos da esquerda e da direita são testados em primeiro lugar, visto que é mais provável esses testes falharem.

#figure[```cpp
bool AABB::isOnOrForwardPlane(const Plane &plane) const
{
    Vec3f center = (max + min) * 0.5;
    Vec3f extents = max - center;

    const float r = extents.x * std::abs(plane.normal.x)
        + extents.y * std::abs(plane.normal.y) 
        + extents.z * std::abs(plane.normal.z);

    return -r <= plane.getSignedDistanceToPlane(center);
}
```]

Para a função `isOnOrForwardPlane`, esta começa por calcular o centro do AABB e o vetor que o centro faz com o máximo. De seguida, calcula o raio do AABB, `r` - basicamente, a projeção do vetor `extents` no vetor normal do plano. Por fim, verifica se o raio é menor ou igual à distância do centro do AABB ao plano, `plane.getSignedDistanceToPlane(center)`. Uma explicação mais profunda pode ser encontrada #link("https://gdbooks.gitbooks.io/3dcollisions/content/Chapter2/static_aabb_plane.html")[neste artigo].

== Visualização de AABBs

Como opção de visualizar e depurar o bom funcionamento desta funcionalidade, é possível renderizar as AABBs nos modelos. Esta opção pode ser ligada/desligada no menu de _settings_ do _ImGui_. As AABBs a serem mostradas, que são as que já tem a matriz de transformação aplicada, têm coordenadas globais e não locais ao modelo. Com isto, como estamos a renderizar a AABB quando fazemos a chamada para renderizar um modelo na cena, temos que resetar a matriz de transformação para a identidade antes de renderizar a AABB. A matriz anterior é reestabelecida depois disso. Isto teve de ser feito já que o cálculo da AABB transformada é feita localmente na chamada de renderização ao modelo e aí já temos a matriz de transformação atual dentro do OpenGL, que iria afetar as coordenadas do AABB a serem renderizadas. Outras opções de implementação implicariam cálcular a AABB transformada duas vezes.

#figure(image("fase4/aabbs simples.png", width: 90%), caption: [Visualização das AABBs dos objetos])

#figure(image("fase4/aabbs.png", width: 90%), caption: [Visualização das AABBs dos objetos no Sistema Solar])

== Testes de performance

Uma vez que o objetivo inicial da implementação do _frustum culling_ era melhorar a performance da _engine_, foram realizados testes de performance para verificar se tal foi, de facto, alcançado.

Nestes testes, a cena utilizada foi a fase final do sistema solar. Foram realizados testes com o _frustum culling_ ativado e desativado, com _V-Sync_ sempre desativado e numa máquina com um processador Ryzen 7 7700X e uma placa gráfica NVIDIA GeForce RTX 2060. Os resultados podem ser observados na seguinte tabela:

#figure[
  #table(
    align: center + horizon,
    columns: 2,
    table.header(
      [*_Frustum Culling_*], [*_FPS_*]
    ),
    [Ativado], [1500],
    [Desativado], [800],
  )
]

Além disso, foi feito um vídeo demonstrativo deste ganho de performance, que pode ser encontrado #link("https://www.youtube.com/watch?v=kBDlUCPjdug")[aqui].

= Sistema solar final

Chegou a hora de dar vida ao sistema solar. Para isso foi adicionado texturas para todos os modelos e agora existe uma luz pontual no centro do sistema solar, representando a luz emitida pelo Sol. Alguns parâmetros de distância e _scaling_ também foram alterados.

== Texturas dos planetas

Para associar a textura do planeta ao planeta em si, foi adicionado uma nova coluna no `.csv` de planetas para o ficheiro de textura do planeta.

#grid(columns: 3, rows: 2, column-gutter: 10pt, row-gutter: 15pt,
  figure(image("fase4/2k_sun.jpg"), caption: [Textura do Sol]),
  figure(image("fase4/8k_earth_daymap.jpg"), caption: [Textura da Terra]),
  figure(image("fase4/2k_moon.jpg"), caption: [Textura da Lua]),
  figure(image("fase4/2k_saturn.jpg"), caption: [Textura de Saturno]),
  figure(image("fase4/2k_mars.jpg"), caption: [Textura de Marte]),
  figure(image("fase4/2k_jupiter.jpg"), caption: [Textura de Jupiter]),
)
\
O mesmo foi feito para os satélites dos planetas, mas a coluna tem valor opcional, visto que não temos texturas para todos os satélites #footnote[A lua (da terra) é a única que acaba por ter uma textura única.]. Quando o satélite não tem uma textura definida, ela é escolhida aleatoriamente de um grupo de texturas de ficção de planetas.

#grid(columns: 3, rows: 2, column-gutter: 10pt, row-gutter: 15pt,
  figure(image("fase4/2k_ceres_fictional.jpg"), caption: [Textura de ficção 1]),
  figure(image("fase4/2k_eris_fictional.jpg"), caption: [Textura de ficção 2]),
  figure(image("fase4/2k_haumea_fictional.jpg"), caption: [Textura de ficção 3]),
) <fictional>

Para os asteroides, foi escolhido a Textura de ficção 3 para todos eles, e o cometa já tem uma textura única.

== Luzes do sistema solar

Para as luzes do sistema solar, é colocado uma luz pontual no centro do sistema solar para simular a iluminação do sol. Os materiais de todos os planetas, asteroides e cometas são o que está de padrão, contudo, como o Sol não é iluminado pela luz pontual do centro, a sua cor emissiva é branca, dando assim o efeito que está a emitir luz.

== Resultado final

Com o _frustum culling_ implementado, o sistema solar final é renderizado com uma performance muito melhor. Com isso, também aumentamos o número de asteroides para 500. Também foi alterado alguns parâmetros para melhorar a perceção do sistema solar e adicionado uma _skybox_ com fundo do espaço #footnote[Este _skybox_ é feito a partir de uma esfera com escalas negativas.].

#block(breakable: false)[
Com todas estas alterações, o resultado final é o seguinte:
#figure(image("fase4/solar system.png"), caption: [Sistema Solar final])
]

#figure(image("fase4/solar system terra.png"), caption: [Sistema Solar final (Vista terra)])

= Editor no _ImGui_ Final

Algumas alterações foram feitas no menu do _ImGui_ nesta fase final. Para além das funcionalidades mostradas anteriormente, o editor foi extendido com mais customizações.

#figure(image("fase4/editor imgui.png"), caption: [_ImGui_ Abas Principais])

Agora é também é apresentado uma tabela com as normais e de coordenadas de texturas de cada modelo.

#figure(image("fase4/editor imgui models.png"), caption: [_ImGui_ Tabela de Modelos])

Também é possível adicionar modelos #footnote[Só é possível adicionar modelos que estejam carregados na _engine_.] à lista de modelos de um grupo, também como remover existentes. Adicionar novos grupos dentro de grupos e remover grupos também é possível em tempo real.

#figure(image("fase4/editor imgui add model.png"), caption: [_ImGui_ Vista de Modelo (Adicionar Modelo)])

A aba de _settings_ também foi remodelada para ficar mais organizada com a grande quantidade de customizações que a _engine_ tem. É possível ligar/desligar luzes, VSync, dizer se é o OpenGL deve fazer _cull_ às faces traseiras, ligar/desligar o modo de wireframe, ligar/desligar MSSA. Para conteúdo adicional de renderização, é possível alterar a cor do background (chamada ao `glClearColor()`), mostrar/esconder o caminho das transformações de translação temporais, mostrar/esconder modelos das luzes apresentado num capítulo anterior, mostrar/esconder os eixos, mostrar/esconder normais dos modelos e mostrar/esconder as AABBs dos modelos. Para comparação de performance, também é possível ligar/desligar o _frustum culling_. Com ele desativado, não é feito qualquer cálculo de AABBs nem de planos do _frustum_.

#figure(image("fase4/editor imgui settings.png"), caption: [_ImGui_ Vista de _Settings_])

= Conclusão

Com o término desta fase, a _engine_ encontra-se finalizada no que toca aos eixos da UC de Computação Gráfica. Foram implementadas todas as funcionalidades pedidas, com algumas extensões, extras e melhorias. A _engine_ é capaz de carregar modelos, texturas, luzes, materiais, e de aplicar transformações a estes. Para além disso, é possível usufruir de uma infinidade de configurações no menu do _ImGui_, que permite ao utilizador customizar a sua experiência de uso.

== Extras

Esta secção é dedicada a todas as funcionalidades que foram implementadas como extra ao solicitado pelo enunciado do projeto.

=== Primitivas

Foi adicionada uma primitiva nova, o cilindro. A geração desta primitiva foi detalhada na primeira fase.

=== Ficheiros _wavefront_ `.obj`

Para além de ser possível definir modelos através de ficheiros `.3d`, também é possível defini-los através de ficheiros `.obj`. Estes ficheiros permitem que seja facilmente renderizado um modelo criado em aplicações como o Blender. O _parsing_ destes ficheiros foi detalhado na primeira fase.

=== Modo de câmara em primeira pessoa com suavização de movimento

Foi adicionado um modo de câmara em primeira pessoa, que permite ao utilizador navegar pela cena livremente, com direito à suavização do seu movimento. Este modo é ativado ao pressionar a tecla `V`. A matemática e respetiva implementação por trás deste modo foi detalhada na segunda fase.

=== _Reload_ de um mundo

É possível ler, em _runtime_ um ficheiro `.xml` que contém a descrição de um mundo e carregar esse mundo. Isto permite que o utilizador possa voltar ao estado inicial da cena que escolheu carregar aquando da execução do mundo. A implementação desta funcionalidade foi detalhada na segunda fase.

=== VBOs com índices

Para melhorar a performance da _engine_, foi implementado o uso de VBOs com índices. A implementação desta funcionalidade foi detalhada na terceira fase e o seu impacto positivo na performance também.

=== Frustum Culling

Tal como no caso anterior, com vista a melhorar a performance, foi implementado _frustum culling_. A implementação desta funcionalidade foi detalhada nesta fase e o seu impacto positivo na performance também.

=== Menu do _ImGui_

Ao longo do desenvolvimento deste projeto, o menu do _ImGui_ foi sendo melhorado, chegando a um estado em que a customização é imensa. 

Segue a lista de funcionalidades que foram adicionadas ao longo do projeto:

- Ativar e desativar: luzes, VSync, _culling_ de faces traseiras, modo de wireframe, MSSA, _frustum culling_, eixos, normais, AABBs, caminho de translação temporal e modelos de luzes.
- Configurar simulação: pausar/continuar e alterar a velocidade de simulação.
- Configurar a câmara: alterar a sua posição, direção, _up_, _fov_, _near_ e _far_; aquando de em modo de primeira pessoa: alterar a velocidade de movimento, de _scrolling_, a aceleração e a fricção.
- Luzes: listar todas as luzes, adicionar e remover luzes.
- Grupos: listar todos os grupos de modelos, adicionar e remover grupos.
- Modelos: listar todos os modelos, adicionar e remover; alterar as transformações de um modelo e entre modelos; alterar a textura de um modelo e o seu modelo; listar vértices e índices de um modelo.

== Trabalho Futuro

=== _Gizmo_

Adicionar um _gizmo_, um mecanismo que permite ao utilizador alterar a posição, rotação e escala de um modelo em tempo real de forma mais intuitiva, arrastando o objeto na cena.

=== Luzes com cores RGB

Atualmente, as luzes têm uma cor branca. Seria interessante adicionar a possibilidade de alterar a cor das luzes para qualquer cor RGB.

=== Coordenadas de texturas mais especializadas

Atualmente, as coordenadas de texturas são estáticas e seguem sempre o mesmo formato. Seria interessante adicionar a possibilidade de alterar as coordenadas de texturas de forma mais específica, por parte do utilizador.

Além disso, seria interessante, também, adicionar a possibilidade de configurar uma textura diferente para cada uma das faces de um cubo ou para a base e face lateral de um cone, tal como acontece com o cilindro.

=== Carregar modelos em tempo real

Seria interessante adicionar a possibilidade de carregar modelos em tempo real, sem ser necessário reiniciar a _engine_.

=== Carregar texturas em tempo real

Da mesma forma que o ponto anterior, seria interessante adicionar a possibilidade de carregar texturas em tempo real, sem ser necessário reiniciar a _engine_.

=== Guardar scenes

A possibilidade de guardar _scenes_ em _runtime_ seria mesmo muito interessante. Isto permitiria ao utilizador guardar o estado atual da cena e carregá-lo posteriormente.
