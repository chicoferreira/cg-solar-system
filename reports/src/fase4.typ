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


= Luzes

Como grande objetivo desta fase, foram implementadas luzes na engine. Como pedido, foram implementadas 3 tipos de luzes: luz pontual, direcional e de foco. As luzes foram feitas usando a implementação padrão de luzes do OpenGL. 

== Normais de Modelos

Para a implementação das luzes, foi necessário calcular as normais dos modelos. Para tal, o programa gerador foi alterado para também calcular as normais dos modelos que ele gera. As normais de um modelo são carregadas para a GPU no início do programa para um _buffer_ prórpio.

=== Normais do Plano

O plano acabou por ser o modelo mais simples de se calcular as normais. Como é um plano, as normais são todas iguais e apontam todas para cima. Portanto, para cada vértice do plano, a normal é (0, 1, 0).

=== Normais da Esfera

Para a esfera também acaba por ser simples. Para cada vértice da esfera, a normal é o vetor que liga o centro da esfera ao vértice. Como a normal deve ser um vetor unitário, calcula-se o vetor com coordenadas esféricas com os mesmos parâmetros do vértice mas com raio 1.

=== Normais do Cubo

Como a nossa implementação do nosso cubo é feita a partir da geração do plano, em que para cada face, é aplicado uma matriz que coloca o plano na posição da face correta, bastou-se aplicar a mesma transformação às normais que já tínhamos calculado para o plano. Isto surgiu ser um problema devido a também termos translações do plano nessa mesma transformação. Ora, obviamente não queremos que a translação do plano seja adicionado à normal. Desta forma teve que se separar a transformação em duas, uma só com rotações e uma só com translações, e só aplicar as transformações de rotação no cálculo das normais.

=== Normais do Cilindro

As normais do cilindro já são um pouco mais complicadas, devido a haver três partes diferentes do cilindro: laterais, base superior e base inferior. Para as laterais, igualmente como na esfera, a normal é o vetor que liga o centro do cilindro com altura do vértice ao vértice, mas com raio 1. Para as bases, a normal para todos os vértices é simplesmente o vetor (0, 1, 0) para a base superior e (0, -1, 0) para a base inferior.

Devido à base ter uma descontinuidade em relação às laterais, apesar das coordenadas dos pontos que os unem serem o mesmo, dois vértices tiveram que ser colocados na mesma posição, um com a normal da base e outro com a normal das laterais. Isto desfaz a optimização que foi feita nas fases anteriores de não repetir vértices entre os vértices que unem as laterais com as bases. #footnote[O mesmo acontecerá com a otimização de não repetir vértices entre o fim da volta completa das laterais e o início da volta seguinte, quando forem implementadas texturas.]

=== Normais do Cone

O cone, em semelhança ao cilindro, também é composto por mais de uma parte: a base e a lateral. A base do cone é igual à base do cilindro, para cada vértice da base, a sua normal é (0, -1, 0). Para as laterais já é feito alguns cálculos. Inicialmente é calculado o ângulo que a superficie lateral faz com um vetor que vai do topo do cone até à base com $alpha = tan^(-1)(r/h)$. Por semelhança de triângulos, este ângulo será o ângulo que a normal faz com a base. 

#figure(image("fase4/cone.png", width: 30%), caption: [Demonstração do cálculo da normal do cone #footnote[Retirado de: https://stackoverflow.com/questions/19245363/opengl-glut-surface-normals-of-cone.]])

Com isto, tendo o ângulo horizontal já cálculado das outras fases podemos calcular as normais usando coordendas esféricas com: $cal(S)(italic("circleAngle"), alpha, 1)$, sendo $cal(S)(alpha, beta, r)$ a função que devolve a coordenada cartesiana a partir de coordenadas esféricas $(alpha,beta,r)$.

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

Para isto, aquando a renderização de um modelo, para cada vértice de modelo é feita uma linha #footnote[Recorrendo ao `GL_LINES` do _OpenGL_.] desde esse vértice até ao vértice do modelo somado com o vetor da normal desse vértice.

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

Os parâmetros das luzes também podem ser configuradas em tempo real a partir do _ImGui_ a partir de um menu próprio das luzes.

#figure(image("fase4/luzes imgui.png"), caption: [Menu de luzes no _ImGui_])

Também é possível remover e adicionar novas luzes.

#figure(image("fase4/luzes adicionar imgui.png"), caption: [Adicionar uma nova luz no _ImGui_])

Como estamos a usar a implementação padrão do _OpenGL_, e esta só suporta até 8 luzes em simultâneo, um aviso é colocado no menu quando o utilizador tem mais de 8 luzes adicionadas em simultâneo. O comportamento da engine neste caso é ignorar as luzes a partir da oitava.

#figure(image("fase4/luzes imgui oito.png"), caption: [Aviso de mais de 8 luzes no _ImGui_])

== Materiais

Como foram implementadas luzes, também teve que ser adicionado um material aos modelos de um grupo. O material é composto por 4 cores diferentes: ambiente, difusa, especular e emissiva, e um coeficiente de brilho. A implementação do material seguiu os requisitos do enunciado. A forma de como a representação dos modelos foi alterada e a sua integração no _ImGui_ será explicada num próximo capítulo.

= Texturas de Modelos

Outra grande requisito desta fase é adicionar a funcionalidade de modelos poderem ter uma textura. As texturas são carregadas no início do programa para um buffer de texturas do _OpenGL_ e são associadas a um modelo através de um índice.

== Coordenadas de Textura

Para começar a implementação de texturas, foi necessário especificar de como as texturas são aplicadas aos modelos. Para tal, foi necessário adicionar um novo vetor de coordenadas de textura a cada vértice de um modelo. Este vetor é um vetor de 2 dimensões que varia entre 0 e 1. As coordenadas de textura são enviadas para a GPU para um _buffer_ próprio no início do programa para cada modelo.

=== Plano

Para o plano, as coordenadas de textura são simples. Para cada vértice, a coordenada de textura é a iteração vertical e horizontal do vértice dividido pelo número de iterações.

Como o plano começa a ser gerado pelo topo esquerdo, as coordenadas de textura são geradas com o $z$ invertido ($z_n = 1.0 - z_i$) de forma a que o topo esquerdo tenha coordenadas de textura (0, 1). 

=== Esfera

Para a esfera, as coordenadas de textura são semelhantes à do plano. Neste caso, a iteração atual do _stack_ e do _slice_ são divididos pelo número de _stacks_ e _slices_ respetivamente. Como começámos a gerar a esfera pela base, as coordenadas de textura não tiveram que ser invertidas como no plano.

Com as coordenadas de textura, muitas otimizações de agrupamento de vértices tiveram que ser retiradas. A otimização de juntar os vértices do fim da volta com o início da volta teve que ser retirado, devido a que, nessa junção tinha que ter duas coordenadas de textura diferentes, no fim $(1,y)$ e no início $(0,y)$. 

Da mesma forma, para os vértices da base e do topo, visto que os vários triângulos que uniam o topo/base ao resto da esfera têm coordenadas de textura diferentes, a otimização de os juntar também teve que ser removida.

=== Cubo

Para o cubo, como estamos a gerá-lo a partir de transformações do plano, as coordenadas de textura são iguais às do plano. No entanto teve que se ter atenção às transformações de rotação que estavam a ser aplicadas para que o cubo apresentasse as texturas com a orientação correta.

Não foi feita qualquer especialização de textura para o cubo. Para aproveitar a textura fornecida pelos docentes para o teste 4.6, a textura é repetida em todas as faces. 

=== Cilindro

Para o cilindro, as coordenadas de textura e o formato das texturas é igual ao que foi feito na ficha prática 11. A imagem da textura é dividida em 3 partes: o corpo a base e o topo.

O mapeamento da textura pode ser visto na imagem seguinte:

#figure(image("fase4/coordenadas textura cilindro.png", width: 40%), caption: [Mapeamento da textura do cilindro])

A sua explicação pode ser vista com grande detalhe nos slides fornecidos pelos docentes da ficha prática 11.

Também como a esfera, as otimizações de juntar vértices do fim da volta com o início da volta e do topo com os triângulos que o unem à lateral tiveram que ser retiradas, já que as coordenadas de textura são diferentes entre o início e o fim da volta das laterais.

=== Cone

Para o cone, as coordenadas de textura são semelhantes às da esfera, em que, para cada _stack_ e _slice_ a sua coordenada de textura é $(italic("slice") / italic("slices"), italic("stacks") / italic("stacks"))$. Isto irá fazer com que a textura fique mapeada à volta da lateral do cone. A otimização de usar os mesmos vértices do início da lateral com o fim da lateral também teve que ser retirada pelos mesmos motivos que foram citados para outros modelos.

Para a textura da base do cone, como não fizemos nenhuma especialização do formato da imagem desta textura, as coordenadas de textura representarão apenas um círculo centrada no centro da textura original, calculada a partir de: $(0.5, 0.5) + cal(P)(0.5, (italic("slice") times 2 pi)/"slices")$, com $cal(P)(r, alpha)$ sendo o cálculo da coordenada cartesiana a partir de uma coordenada polar.

=== _Bezier Patch_

Para os _patches_, as coordenadas de textura para cada par ($u$,$v$), como $u in [0,1] and v in [0,1]$, são exatamente esse par. Isto faz com que a textura fique mapeada à volta do _patch_ corretamente. A textura é repetida para cada _patch_.

== Formato final dos ficheiros .3d

Para acomudar as novas informações dos modelos (normais e coordenadas de texturas), o formato do ficheiro .3d foi alterado para guardar essas informações. Agora as normais e coordenadas de texturas são guardadas depois da informação dos vértices.

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

Um _preview_ da imagem da textura atual é também é apresentada. Clicando na nome da textura, um _dropdown_ é aberto e é possível escolher qualquer outra textura que esteja carregada no programa.

#figure(image("fase4/model imgui textura.png", width: 90%), caption: [Menu de um modelo no _ImGui_ (Mudar textura)])

#block(breakable: false)[
Ao clicar na cor de cada um dos componentes é possível abrir um menu de _Color Picker_, onde a cor pode ser escolhida com mais detalhe.

#figure(image("fase4/model imgui material.png", width: 90%), caption: [Menu de um modelo no _ImGui_ (_Color Picker_)])
]


= Frustum Culling

https://github.com/chicoferreira/cg-solar-system/commit/b971ab91d45633a595922168f3b7d6062c9c6942

== Cálculo do AABB

== Cálculo do Frustum

== Verificação de Interseção

= Sistema solar final

== Texturas dos planetas

== Luzes do sistema solar

Sol tem cor emissiva e há uma point light na origem do sistema solar

== Resultado final

Foi possível aumentar em grande quantidade o número de asteroides na cintura com o frustum culling
Skybox

= Editor no _ImGui_ Final




Agora é possível adicionar grupos e remover, também como adicionar modelos. Limitação de só funcionar com modelos já carregados.

Background color

Nova aba de settings

= Conclusão

Mostrar todos os extras

Ainda há muito a fazer mas o resultado final é muito bom

Luzes com cores RGB

Coordenadas de texturas mais especificas

Carregar modelos em tempo real

Carregar texturas em tempo real

Guardar scenes

Lista de todos os extras