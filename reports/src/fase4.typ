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

Luzes são ligadas se o xml tiver o parametro \<lights>

== Normais de Modelos

=== Normais do Plano

=== Normais da Esfera

=== Normais do Cubo

=== Normais do Cone

=== Normais do Cilindro

=== Normais das Patch

https://github.com/chicoferreira/cg-solar-system/commit/cc3ce94b6d215fd727e15b0a61b3449ab98719c7

== Modelo de luzes

Toggle no ImGui

== Integração com o ImGui

(Aviso se colocar mais de 8 luzes)

= Texturas de Modelos

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