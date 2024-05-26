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

Para a implementação das luzes, foi necessário calcular as normais dos modelos. Para tal, o programa gerador foi alterado para também calcular as normais dos modelos que ele gera.

=== Normais do Plano

O plano acabou por ser o modelo mais simples de se calcular as normais. Como é um plano, as normais são todass iguais e apontam todas para cima. Portanto, para cada vértice do plano, a normal é (0, 1, 0).

=== Normais da Esfera

Para a esfera também acaba por ser simples. Para cada vértice da esfera, a normal é o vetor que liga o centro da esfera ao vértice. Como a normal deve ser um vetor unitário, calcula-se o vetor com coordenadas esféricas com os mesmos parâmetros do vértice mas com raio 1.

=== Normais do Cubo

Como a nossa implementação do nosso cubo é feita a partir da geração do plano, em que para cada face, é aplicado uma matriz que coloca o plano na posição da face correta, bastou-se aplicar a mesma transformação às normais que já tínhamos calculado para o plano.

=== Normais do Cilindro

As normais do cilindro já são um pouco mais complicadas, devido a haver três partes diferentes do cilindro: laterais, base superior e base inferior. Para as laterais, igualmente como na esfera, a normal é o vetor que liga o centro do cilindro com altura do vértice ao vértice, mas com raio 1. Para as bases, a normal para todos os vértices é simplesmente o vetor (0, 1, 0) para a base superior e (0, -1, 0) para a base inferior.

Devido à base ter uma descontinuidade em relação às laterais, apesar das coordenadas dos pontos que os unem serem o mesmo, dois vértices tiveram que ser colocados na mesma posição, um com a normal da base e outro com a normal das laterais. Isto desfaz a optimização que foi feita nas fases anteriores de não repetir vértices entre os vértices que unem as laterais com as bases. #footnote[O mesmo acontecerá com a otimização de não repetir vértices entre o fim da volta completa das laterais e o início da volta seguinte, quando forem implementadas texturas.]

=== Normais do Cone

O cone, em semelhança ao cilindro, também é composto por mais de uma parte: a base e a lateral. A base do cone é igual à base do cilindro, para cada vértice da base, a sua normal é (0, -1, 0). Para as laterais já é feito alguns cálculos. Inicialmente é calculado o ângulo que a superficie lateral faz com um vetor que vai do topo do cone até à base com $alpha = tan^(-1)(r/h)$. Por semelhança de triângulos, este ângulo será o ângulo que a normal faz com a base. 

#figure(image("fase4/cone.png", width: 30%), caption: [Demonstração do cálculo da normal do cone #footnote[Retirado de: https://stackoverflow.com/questions/19245363/opengl-glut-surface-normals-of-cone.]])

Com isto, tendo o ângulo horizontal já cálculado das outras fases podemos calcular as normais usando coordendas esféricas com: $cal(S)(italic("circleAngle"), alpha, 1)$, sendo $cal(S)(alpha, beta, r)$ a função que devolve a coordenada cartesiana a partir de coordenadas esféricas $(alpha,beta,r)$.

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


= Texturas de Modelos

Outra grande requisito desta fase é adicionar a funcionalidade de modelos poderem ter uma textura.

== Coordenadas de Textura
=== Plano
=== Esfera
=== Cubo
=== Cone
=== Cilindro
=== Patch


== Formato final dos ficheiros .3d

== Estrutura final de um modelo na engine
Carregamento de texturas...

== Integração com o ImGui

Textura é mostrada no ImGui e é possível trocar a textura por outra carregada

Nova tabela de coordenadas de texturas e normais

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

= Editor no ImGui Final

Agora é possível adicionar grupos e remover, também como adicionar modelos. Limitação de só funcionar com modelos já carregados.

Background color

Nova aba de settings

= Conclusão

Mostrar todos os extras

Ainda há muito a fazer mas o resultado final é muito bom

Luzes com cores RGB