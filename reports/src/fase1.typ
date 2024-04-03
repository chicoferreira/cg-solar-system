#import "template.typ": *

#show: project.with(
  title: "Computação Gráfica - Fase 1",
  date: "5 de Março de 2024",
  group: [*Grupo 2*],
  authors: (
    (name: "Daniel Pereira", number: "A100545"),
    (name: "Duarte Ribeiro", number: "A100764"),
    (name: "Francisco Ferreira", number: "A100660"),
    (name: "Rui Lopes", number: "A100643"),
  ),
)
 
#heading(numbering: none)[Introdução]

Este relatório tem como objetivo apresentar o trabalho prático desenvolvido para a primeira fase da unidade curricular de Computação Gráfica. O trabalho consiste no desenvolvimento de uma _engine_ que carrega cenas com modelos a partir de um ficheiro e a mostra no ecrã, usando várias técnicas sobre o tópico da UC.

= Visão geral da arquitetura do projeto

O projeto encontra-se dividido em dois programas principais: gerador e _engine_.
Este primeiro tem como propósito a geração de figuras primitivas gráficas, resultando em ficheiros `.3d`. Esses ficheiros são, depois, embutidos em ficheiros `.xml`, lidos e renderizados pela _engine_. O funcionamento de cada um destes programas é detalhado ao pormenor nas secções seguintes.

= Geração de figuras primitivas gráficas

Como requisito deste trabalho prático, foi necessário fazer um gerador de primitivas gráficas. Isto consiste num programa que, conforme os argumentos passados para o mesmo, escreve num ficheiro de texto informações necessárias para desenhar a primitiva pretendida no ecrã.

O nosso programa gera ficheiros como resultado, num formato idealizado pelo grupo, que nós apelidamos de ficheiros `.3d`. Estes ficheiros em formato textual seguem o seguinte formato: a primeira linha contém o número de vértices do modelo, $n$, seguido de $n$ vértices tridimensionais com coordenadas separadas por espaço seguindo a ordem $x$, $y$ e $z$, que formam a figura geométrica em questão. Um exemplo de um retângulo a ocupar o ecrã completamente #footnote[Assumindo que não há transformações na câmara (coordenada $z$ ignorada), o centro do ecrã tem as coordenadas $(0,0)$, o canto inferior esquerdo tem as coordenadas $(-1, -1)$ e o canto superior direito tem as coordenadas $(1,1)$.] guardado neste formato seria:
#figure[```
6
-1 1 0
-1 -1 0
1 1 0
1 1 0
-1 -1 0
1 -1 0
```] <3d_file>

A forma como este ficheiro é lido para memória no programa da _engine_ é explicada no #link(<parsing>)[próximo capítulo].

Foi optado pelo formato textual e não por um formato binário, que poderia trazer benefícios de rapidez de leitura e melhor utilização de espaço de disco, devido à conveniência de leitura e alteração de dados por parte de um utilizador, que assim, não tem de recorrer a programas que interpretariam o formato binário para o editar. Desta forma, um utilizador poderá apenas editar o ficheiro em forma de texto e poderá alterar ou adicionar novos vértices muito facilmente.

Nas fases seguintes, este formato será estendido e alterado de modo a guardar índices de faces, vetores normais, informação de texturas, entre outros possíveis requisitos dessas.
Para já, achamos que temos um formato adequado para esta fase.

== As figuras

Nesta fase houve como requisito o desenvolvimento da geração das seguintes primitivas: #link(<plane>)[Plano], #link(<esfera>)[Esfera], #link(<cone>)[Cone] e #link(<box>)[Caixa]. Como extra, também desenvolvemos a geração da primitiva #link(<cylinder>)[Cilindro].

A forma como um utilizador pode gerar estas figuras está auto-descrita na mensagem de _help_ do gerador:

```bash
Usage: generator <command> <args> <output file>
Commands:
        generator plane <length> <divisions> <output file>
        generator sphere <radius> <slices> <stacks> <output file>
        generator cone <radius> <height> <slices> <stacks> <output file>
        generator box <length> <divisions> <output file>
        generator cylinder <radius> <height> <slices> <output file>
```

Segue-se a descrição de como foram trianguladas cada uma dessas figuras.

#import "fase1/3d.typ": *

=== Geração do plano <plane>

- Parâmetros: Tamanho (decimal), Divisões (inteiro);

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

    let origin = point(..project(0,0,0, size))
    origin
  })
}


#table(columns: 2, stroke: none,
  figure(generate_plane(length: 1.4, divisions: 3, iteration: 0), caption: [Primeira iteração da \ geração de planos #footnote[Os eixos seguem a seguinte coloração: (#text(fill:red)[$x$], #text(fill:green)[$y$], #text(fill:blue)[$z$])]]),
  figure(generate_plane(length: 1.4, divisions: 3, iteration: 1), caption: [Segunda iteração da \ geração de planos]),
)

As triangulações são feitas da forma como estão descritas nas figuras acima. O tamanho é dividido pelo número de divisões, e assim obtemos o espaçamento entre os pontos. Os pontos $P_1$, $P_2$, $P_3$ e $P_4$ são obtidos da seguinte forma:

$
P_1 = (-l/2 + x times s, 0, -l/2 + z times s), forall x,z in {0, 1, ..., d}\
P_2 = (-l/2 + x times s, 0, -l/2 + z times s), forall x,z in {0, 1, ..., d}\
P_3 = (-l/2 + x times s, 0, -l/2 + z times s), forall x,z in {0, 1, ..., d}\
P_4 = (-l/2 + x times s, 0, -l/2 + z times s), forall x,z in {0, 1, ..., d}
$

Sendo, $l$ o tamanho do plano, $s$ o tamanho de cada subdivisão (igual a $l/d$) e $d$ o número de divisões.

A cada iteração, estes pontos são colocados como resultado pela ordem: ${P_1, P_3, P_4} + {P_1, P_4, P_2}$, formando os dois triângulos de cada subdivisão com normal paralela ao eixo $y$ (para ser vista de cima).

=== Geração da Esfera <esfera>
- Parâmetros: Raio (decimal), _Slices_ (inteiro), _Stacks_ (inteiro);

#let generate_sphere(radius: 0.7, slices: 10, stacks: 10, iteration: (0,0), hide_points: ()) = {
  let slice_size = 2 * calc.pi / slices;
  let stack_size = calc.pi / stacks;

  let size = 200
  let h_size = 190

  let origin = (size / 2, size / 2)
  
  box(width: size * 1pt, height: h_size * 1pt, {
    place(line(stroke: red, start: to_pt(origin), end: to_pt(project(1,0,0, size))))
    place(line(stroke: green, start: to_pt(origin), end: to_pt(project(0,1,0, size))))
    place(line(stroke: blue, start: to_pt(origin), end: to_pt(project(0,0,1, size))))

    let points = ()

    for slice in range(0, slices) {
      for stack in range(0, stacks) {
        let top_left = project(..spherical_to_cartesian(radius, slice * slice_size - calc.pi/2, stack * stack_size + calc.pi/2), size);
        let top_right = project(..spherical_to_cartesian(radius, (slice + 1) * slice_size - calc.pi/2, stack * stack_size+ calc.pi/2), size);
        let bottom_left = project(..spherical_to_cartesian(radius, slice * slice_size - calc.pi/2 , (stack + 1) * stack_size+ calc.pi/2), size);
        let bottom_right = project(..spherical_to_cartesian(radius, (slice + 1) * slice_size - calc.pi/2, (stack + 1) * stack_size+ calc.pi/2), size);

        set line(stroke: gray.darken(20%))
         
        if is_triangle_ccw(top_left, bottom_left, bottom_right) { 
          place(line(start: to_pt(top_left), end: to_pt(bottom_left)))
          place(line(start: to_pt(bottom_left), end: to_pt(bottom_right)))
          place(line(start: to_pt(top_left), end: to_pt(top_right)))
          place(line(start: to_pt(top_right), end: to_pt(bottom_right)))
          place(line(start: to_pt(top_left), end: to_pt(bottom_right)))
        }

        if(slice == iteration.at(0) and stack == iteration.at(1)) {
          points += (top_left, top_right, bottom_left, bottom_right)
        }
      }
    }

    for (i, p) in points.enumerate() {
      if hide_points.contains(i + 1) {
        continue
      }

      point(..to_pt(p))
      
      place(dx: p.at(0) * 1pt, dy: p.at(1) * 1pt - 11pt, [$P_#(i + 1)$])
    }

  })
}

#table(columns: 2, stroke: none,
  figure(generate_sphere(iteration: (7,4)), caption: [Uma iteração da \ geração de esferas]),
  figure(generate_sphere(iteration: (8,4)), caption: [Próxima iteração da \ geração de esferas]),
)

A geração de esferas tem o mesmo princípio da geração dos planos, mas, neste caso, é usado coordenadas esféricas para este efeito. No caso, a cada iteração é calculado um novo ângulo para a posição dos pontos.

A conversão de coordenadas esféricas para coordenadas cartesianas é dada pela fórmula:
#math.equation(block: true, numbering: "(1)", $
cal(S)(r, alpha, beta) = (r cos(beta) sin(alpha), r sin(beta), r cos(beta) cos(alpha))
$) <spherical>

Com isto, para gerar a esfera, podemos fazer com que $alpha$ incremente $(2pi)/"slices"$ a cada iteração e que $beta$ incremente $pi/"stacks"$ a cada iteração do $alpha$. O número de iterações totais é dado por $"slices" times "stacks"$ vezes. O $r$ é dado pelo parâmetro "Raio" da esfera.

Assim, os pontos $P_1$, $P_2$, $P_3$ e $P_4$ são obtidos da seguinte forma:

$
P_1 &= cal(S)(r, "slice" times (2pi)/"slices", ("stack" + 1) times pi/"stacks" - pi/2)\
P_2 &= cal(S)(r, ("slice" + 1) times (2pi)/"slices", ("stack" + 1) times pi/"stacks" - pi/2)\
P_3 &= cal(S)(r, "slice" times (2pi)/"slices", "stack" times pi/"stacks" - pi/2)\
P_4 &= cal(S)(r, ("slice" + 1) times (2pi)/"slices", "stack" times pi/"stacks" - pi/2) 
$

Sendo $"slice"$ a iteração do _slice_ atual, $"stack"$ a iteração da _stack_ atual e $cal(S)$ a função em @spherical.

A razão de se subtrair $pi/2$ é relacionado à necessidade de "rodar" a esfera, sobre o eixo do $z$, noventa graus, devido à nossa definição do $cal(S)$ (@spherical).

Os pontos são adicionados ao resultado na mesma ordem do que o plano: ${P_1, P_3, P_4} + {P_1, P_4, P_2}$.

==== Problema dos polos da esfera <sphere_problem>

#figure(generate_sphere(iteration: (8,0), hide_points: (2,)), caption: [Iteração no polo superior na geração da esfera])

No caso demonstrado na figura acima, $P_1$ e $P_2$ partilham as mesmas coordenadas devido a estarem num polo da esfera. Usando a mesma lógica de fazer a triangulação da forma ${P_1, P_3, P_4} + {P_1, P_4, P_2}$, o segundo triângulo vai ser apenas uma linha. Para evitar este triângulo inválido, apenas a primeira triangulação é adicionada, quando a iteração está num dos polos.

Este problema também acontece na #link(<cone_problem>)[Geração do Cone].

=== Geração do Cone <cone>

- Parâmetros: Raio (decimal), Altura (decimal), _Slices_ (inteiro), _Stacks_ (inteiro);

#let generate_cone(radius: 0.5, height: 1, slices: 10, stacks: 7, iteration: (0,0), hide_points: ()) = {
  let slice_size = 2 * calc.pi / slices;
  let stack_size = height / stacks;

  let size = 200
  let h_size = 150

  let origin = (size / 2, size / 2)
  
  box(width: size * 1pt, height: h_size * 1pt, {
    place(line(stroke: red, start: to_pt(origin), end: to_pt(project(1,0,0, size))))
    place(line(stroke: green, start: to_pt(origin), end: to_pt(project(0,1,0, size))))
    place(line(stroke: blue, start: to_pt(origin), end: to_pt(project(0,0,1, size))))

    let points = ()

    for slice in range(0, slices) {
      for stack in range(0, stacks) {
        let current_radius = radius - stack * radius / stacks;
        let next_radius = radius - (stack + 1) * radius / stacks;
        
        let bottom_left = project(..polar_to_cartesian(current_radius, slice * slice_size, stack * stack_size), size)
        let bottom_right = project(..polar_to_cartesian(current_radius, (slice + 1) * slice_size, stack * stack_size), size)
        let top_left = project(..polar_to_cartesian(next_radius, slice * slice_size, (stack + 1) * stack_size), size)
        let top_right = project(..polar_to_cartesian(next_radius, (slice + 1) * slice_size, (stack + 1) * stack_size), size)


        set line(stroke: gray.darken(20%))
         
        if is_triangle_ccw(top_left, bottom_left, bottom_right) { 
          place(line(start: to_pt(top_left), end: to_pt(bottom_left)))
          place(line(start: to_pt(bottom_left), end: to_pt(bottom_right)))
          place(line(start: to_pt(top_left), end: to_pt(top_right)))
          place(line(start: to_pt(top_right), end: to_pt(bottom_right)))
          place(line(start: to_pt(top_left), end: to_pt(bottom_right)))
        }

        if(slice == iteration.at(0) and stack == iteration.at(1)) {
          points += (top_left, top_right, bottom_left, bottom_right)
        }
      }
    }

    for (i, p) in points.enumerate() {
      if hide_points.contains(i + 1) {
        continue
      }
      let p = to_pt(p)

      point(..p)
      
      place(dx: p.at(0), dy: p.at(1) - 11pt, [$P_#(i + 1)$])
    }

  })
}

#table(columns: 2, stroke: none,
  figure(generate_cone(iteration: (0,0)), caption: [Primeira iteração da \ geração de cones]),
  figure(generate_cone(iteration: (0,1)), caption: [Segunda iteração da \ geração de cones]),
)

Similarmente às figuras anteriores, o cone usa o mesmo método de iterações sobre _slices_ e _stacks_ da esfera, mas a cada _stack_ (divisão horizontal), calcula os seus pontos a partir de coordenadas polares. O componente do raio faz uma interpolação linear desde a base até ao topo, tendo aí o valor $0$, dando o efeito do cone.

A conversão de coordenadas polares para coordenadas cartesianas é dada pela fórmula:
#math.equation(block: true, numbering: "(1)", $
cal(P)(r, alpha, y) = (r sin(alpha), y, r cos(alpha))
$) <polar>

Usando isto, pode-se facilmente gerar cada _stack_ do cone como um círculo, unindo com a _stack_ da camada superior.

Com a mesma lógica e ordem dos pontos das figuras geométricas anteriores temos: 

$
l_"slice" &= (2pi)/"slices"\
l_"stack" &= h/"stacks"\
r_i &= r - "stack" times  l_"stack"\
r_(i+1) &= r - ("stack" + 1) times  l_"stack"\
P_1 &= cal(P)(r_(i+1), "slice" times l_"slice", ("stack" + 1) times l_"stack")\
P_2 &= cal(P)(r_(i+1), ("slice" + 1) times l_"slice", ("stack" + 1) times l_"stack")\
P_3 &= cal(P)(r_i, "slice" times l_"slice", "stack" times l_"stack")\
P_4 &= cal(P)(r_i, ("slice" + 1) times l_"slice", "stack" times l_"stack") 
$

Sendo $"slice"$ o _slice_ da iteração atual, $"stack"$ o _stack_ da iteração atual, $h$ a altura do cone, $"stacks"$ o número de _stacks_ do cone, $"slices"$ o número de _slices_ do cone e $r$ o raio do cone.

Os pontos são adicionados ao resultado na mesma ordem do que a esfera: ${P_1, P_3, P_4} + {P_1, P_4, P_2}$.

==== Base do cone

A geração descrita não é suficiente para completar o cone, visto que a base do cone não é tratada. Para resolver isto, por cada _slice_ há um triângulo a mais que liga deste os vértices da camada da base até ao ponto da origem, que é o centro da base do cone.

#let generate_base_cone(radius: 0.5, height: 1, slices: 10, stacks: 7, iteration: 0, hide_points: ()) = {
  let slice_size = 2 * calc.pi / slices;
  let stack_size = height / stacks;

  let size = 200
  let h_size = 150

  let origin = (size / 2, size / 2)
  
  box(width: size * 1pt, height: h_size * 1pt, {
    place(line(stroke: red, start: to_pt(origin), end: to_pt(project(1,0,0, size))))
    place(line(stroke: green, start: to_pt(origin), end: to_pt(project(0,1,0, size))))
    place(line(stroke: blue, start: to_pt(origin), end: to_pt(project(0,0,1, size))))

    let points = ()

    set line(stroke: gray.darken(20%))
    
    for slice in range(0, slices) {
      for stack in range(0, stacks) {
        let current_radius = radius - stack * radius / stacks;
        let next_radius = radius - (stack + 1) * radius / stacks;
        
        let bottom_left = project(..polar_to_cartesian(current_radius, slice * slice_size, stack * stack_size), size)
        let bottom_right = project(..polar_to_cartesian(current_radius, (slice + 1) * slice_size, stack * stack_size), size)
        let top_left = project(..polar_to_cartesian(next_radius, slice * slice_size, (stack + 1) * stack_size), size)
        let top_right = project(..polar_to_cartesian(next_radius, (slice + 1) * slice_size, (stack + 1) * stack_size), size)

        if is_triangle_ccw(top_left, bottom_left, bottom_right) { 
          place(line(start: to_pt(top_left), end: to_pt(bottom_left)))
          place(line(start: to_pt(bottom_left), end: to_pt(bottom_right)))
          place(line(start: to_pt(top_left), end: to_pt(top_right)))
          place(line(start: to_pt(top_right), end: to_pt(bottom_right)))
          place(line(start: to_pt(top_left), end: to_pt(bottom_right)))
        }
      }
      
      let base_bottom_left = project(..polar_to_cartesian(radius, slice * slice_size, 0), size)
      let base_bottom_right = project(..polar_to_cartesian(radius, (slice + 1) * slice_size, 0), size)

      if(slice == iteration) {      
        place(line(start: to_pt(origin), end: to_pt(base_bottom_left)))
        place(line(start: to_pt(origin), end: to_pt(base_bottom_right)))
  
        points += (origin, base_bottom_right, base_bottom_left)
      }
    }

    for (i, p) in points.enumerate() {
      if hide_points.contains(i + 1) {
        continue
      }
      let p = to_pt(p)

      point(..p)
      
      place(dx: p.at(0), dy: p.at(1) - 11pt, [$P_#(i + 1)$])
    }
  })
}

#figure(generate_base_cone(slices: 8, stacks: 1), caption: [Iteração para a geração da base do cone])

Estes pontos são adicionados ao resultado pela ordem: ${P_1, P_2, P_3}$. Assim, o triângulo fica com normal com vetor "para baixo", mostrando assim a base, apenas quando a câmara o vê por baixo.

==== Problema dos polos do cone <cone_problem>

#figure(generate_cone(iteration: (0,2), hide_points: (2,), stacks: 3), caption: [Iteração no polo superior na geração do cone])

Na mesma forma de ideias da esfera (@sphere_problem), o cone também partilha o mesmo problema de, na iteração no polo superior, o ponto $P_1$ partilhar as mesmas coordenadas do $P_2$. Isto é resolvido da mesma forma, em que não é adicionado o segundo triângulo, ${P_1, P_4, P_2}$, à lista de vértices resultado, neste caso.

=== Geração da Caixa <box>

- Parâmetros: Tamanho (decimal), Divisões (inteiro);

#let generate_box(length: 1, divisions: 3) = {
  let side = length / divisions

  let size = 200pt
  let h_size = 180pt

  let origin = (size / 2, size / 2)
  
  box(width: size, height: h_size, {
    place(line(stroke: red, start: origin, end: project(1,0,0, size)))
    place(line(stroke: green, start: origin, end: project(0,1,0, size)))
    place(line(stroke: blue, start: origin, end: project(0,0,1, size)))

    let points = ()

    for x in range(0, divisions) {
      for z in range(0, divisions) {
        let top_left_up = project(-length / 2 + x * side, length / 2, -length / 2 + z * side, size);
        let top_right_up = project(-length / 2 + (x + 1) * side, length / 2, -length / 2 + z * side, size);
        let bottom_left_up = project(-length / 2 + x * side, length / 2, -length / 2 + (z + 1) * side, size);
        let bottom_right_up = project(-length / 2 + (x + 1) * side, length / 2, -length / 2 + (z + 1) * side, size);

        let top_left_front = project(-length / 2 + x * side, -length / 2 + z * side, length / 2, size);
        let top_right_front = project(-length / 2 + (x + 1) * side, -length / 2 + z * side, length / 2, size);
        let bottom_left_front = project(-length / 2 + x * side, -length / 2 + (z + 1) * side, length / 2, size);
        let bottom_right_front = project(-length / 2 + (x + 1) * side, -length / 2 + (z + 1) * side, length / 2, size);

        let top_left_right = project(length / 2, -length / 2 + x * side, -length / 2 + z * side, size);
        let top_right_right = project(length / 2, -length / 2 + (x + 1) * side, -length / 2 + z * side, size);
        let bottom_left_right = project(length / 2, -length / 2 + x * side, -length / 2 + (z + 1) * side, size);
        let bottom_right_right = project(length / 2, -length / 2 + (x + 1) * side, -length / 2 + (z + 1) * side, size);

        set line(stroke: gray.darken(20%))

        place(line(start: top_left_up, end: bottom_left_up))
        place(line(start: bottom_left_up, end: bottom_right_up))
        place(line(start: top_left_up, end: top_right_up))
        place(line(start: top_right_up, end: bottom_right_up))
        place(line(start: top_left_up, end: bottom_right_up))

        place(line(start: top_left_right, end: bottom_left_right))
        place(line(start: bottom_left_right, end: bottom_right_right))
        place(line(start: top_left_right, end: top_right_right))
        place(line(start: top_right_right, end: bottom_right_right))
        place(line(start: top_left_right, end: bottom_right_right))

        place(line(start: top_left_front, end: bottom_left_front))
        place(line(start: bottom_left_front, end: bottom_right_front))
        place(line(start: top_left_front, end: top_right_front))
        place(line(start: top_right_front, end: bottom_right_front))
        place(line(start: top_left_front, end: bottom_right_front))
      }
    }
  })
}

#figure(generate_box(), caption: [Geração de uma caixa])

Conseguimos notar facilmente que a caixa é composta com várias réplicas (6 faces) do plano. Usando esta noção e tendo o conhecimento de multiplicação de matrizes e como elas conseguem fazer rotações e translações em vetores, podemos chegar à caixa muito facilmente. Podemos aproveitar o código feito para a #link(<plane>)[Geração do Plano] e fazer as tais transformações nos pontos com as matrizes adequadas. Isto já aproveita a lógica de subdivisão dos planos e não temos de lidar com isso.

Iremos mostrar as matrizes que fazem as tais rotações e translações necessárias, que foram aplicadas a todos os pontos gerados pela Geração do Plano.

De notar, que foi adicionado um quarto componente a mais com valor $1$ a todos os pontos, para se conseguirem fazer translações a partir de matrizes.

*Face superior:*

Esta face é simples, visto que apenas é necessário deslocar o plano $l/2$ unidades sobre o eixo $y$, sendo $l$ o tamanho da caixa.

#set math.mat(delim: "[")

$
cal(F)_"up"=mat(1,0,0,0;0,1,0,l/2;0,0,1,0;0,0,0,1) mat(x;y;z;1)
$

*Face inferior:*

A face inferior passa por uma translação de $-l/2$ unidades sobre o eixo $y$ e uma rotação de 180º sobre o eixo $x$, para que face fique virada para baixo (corrigindo assim a sua normal).

#let mat_rotate_x(angle) = $ mat(1, 0, 0, 0; 0, cos(#angle), -sin(#angle), 0 ; 0, sin(#angle), cos(#angle), 0 ; 0, 0, 0, 1) $
#let mat_rotate_y(angle) = $ mat(cos(#angle), 0, sin(#angle), 0; 0, 1, 0, 0 ; -sin(#angle), 0, cos(#angle), 0 ; 0, 0, 0, 1) $
#let mat_rotate_z(angle) = $ mat(cos(#angle), -sin(#angle), 0, 0; sin(#angle), cos(#angle), 0, 0 ; 0, 0, 1, 0 ; 0, 0, 0, 1) $

$
cal(F)_"down" = mat(1,0,0,0;0,1,0,-l/2;0,0,1,0;0,0,0,1) #mat_rotate_x[$pi$] mat(x;y;z;1)
$

*Face frontal:*

Obtém-se a face frontal aplicando uma translação de $l/2$ no eixo do $z$, e uma rotação de 90º sobre o eixo do $x$.

$
cal(F)_"front" = mat(1,0,0,0;0,1,0,0;0,0,1,l/2;0,0,0,1) #mat_rotate_x[$pi/2$] mat(x;y;z;1)
$

*Face da direita:*

A face da direita é calculada fazendo uma translação de $l/2$ no eixo do $x$ e uma rotação de -90º sobre o eixo do $z$.

$
cal(F)_"right" = mat(1,0,0,l/2;0,1,0,0;0,0,1,0;0,0,0,1) #mat_rotate_z[$-pi/2$] mat(x;y;z;1)
$

*Face da esquerda:*

Já a face da esquerda é obtida pelo inverso da face da direita, ou seja, uma translação de $-l/2$ no eixo do $x$ e uma rotação de 90º sobre o eixo do $z$.

$
cal(F)_"left" = mat(1,0,0,-l/2;0,1,0,0;0,0,1,0;0,0,0,1) #mat_rotate_z[$pi/2$] mat(x;y;z;1)
$

*Face traseira:*

Por fim, a face traseira é gerada pelo inverso da face da frente, isto é, uma translação de $-l/2$ no eixo do $z$ e uma rotação de -90º sobre o eixo do $x$.

$
cal(F)_"back" = mat(1,0,0,-l/2;0,1,0,0;0,0,1,0;0,0,0,1) #mat_rotate_x[$pi/2$] mat(x;y;z;1)
$

#h(10pt)

Com isto, os pontos gerados por todas estas faces formam a caixa.

== Geração do Cilindro <cylinder>

- Parâmetros: Raio (decimal), Altura (decimal), _Slices_ (inteiro);

#let generate_cylinder(radius: 0.5, height: 1, slices: 20, iteration: 0, hide_points: ()) = {
  let alpha = 2 * calc.pi / slices;

  let size = 200
  let h_size = 150
  
  let base_middle = project(0, 0, 0, size);
  let up_middle = project(0, height, 0, size);

  let origin = (size / 2, size / 2)

  box(width: size * 1pt, height: h_size * 1pt, {
    place(line(stroke: red, start: to_pt(origin), end: to_pt(project(1,0,0, size))))
    place(line(stroke: green, start: to_pt(origin), end: to_pt(project(0,1,0, size))))
    place(line(stroke: blue, start: to_pt(origin), end: to_pt(project(0,0,1, size))))

    let points = ()

    for slice in range(0, slices) {
      let base_vertex_left = project(..polar_to_cartesian(radius, slice * alpha, 0), size)
      let base_vertex_right = project(..polar_to_cartesian(radius, (slice + 1) * alpha, 0), size)

      let up_vertex_left = project(..polar_to_cartesian(radius, slice * alpha, height), size)
      let up_vertex_right = project(..polar_to_cartesian(radius, (slice + 1) * alpha, height), size)

      set line(stroke: gray.darken(20%))

      place(line(start: to_pt(up_middle), end: to_pt(up_vertex_left)))
      place(line(start: to_pt(up_vertex_left), end: to_pt(up_vertex_right)))
      
      if (is_triangle_ccw(up_vertex_left, base_vertex_left, base_vertex_right)) {        
          place(line(start: to_pt(up_vertex_left), end: to_pt(base_vertex_left)))
          place(line(start: to_pt(base_vertex_left), end: to_pt(base_vertex_right)))
      }

      if is_triangle_ccw(base_vertex_left, base_vertex_right, up_vertex_left) {
          place(line(start: to_pt(base_vertex_left), end: to_pt(base_vertex_right)))
          place(line(start: to_pt(base_vertex_right), end: to_pt(up_vertex_left)))
      }

      if is_triangle_ccw(base_vertex_left, base_vertex_right, up_vertex_right) {
          place(line(start: to_pt(base_vertex_left), end: to_pt(base_vertex_right)))
          place(line(start: to_pt(base_vertex_right), end: to_pt(up_vertex_right)))
      }

      if iteration == slice {
        points += (up_vertex_left, up_vertex_right, base_vertex_left, base_vertex_right)
      }
    }

    for (i, p) in points.enumerate() {
      if hide_points.contains(i + 1) {
        continue
      }
      let p = to_pt(p)
      point(..p)

      place(dx: p.at(0), dy: p.at(-1) - 11pt, [$P_#(i + 1)$])
    }
    
    place(dx: up_middle.at(0) * 1pt, dy: up_middle.at(1) * 1pt - 11pt, [$M$])
    point(..to_pt(up_middle))

  })
}

#table(columns: 2, stroke: none,
  figure(generate_cylinder(iteration: 0), caption: [Primeira iteração da \ geração de cilindros]),
  figure(generate_cylinder(iteration: 1), caption: [Segunda iteração da \ geração de cilindros]),
)

A geração de cilindros é muito similar à #link(<cone>)[Geração do Cone] mas sem a interpolação do raio (mantendo-se sempre igual), tendo uma base a mais no topo e sem divisão de _stacks_.

Com isto, os pontos $P_1$, $P_2$, $P_3$ e $P_4$ são obtidos da seguinte forma:

$
P_1 &= cal(S)(r, "slice" times (2pi)/"slices", h)\
P_2 &= cal(S)(r, ("slice" + 1) times (2pi)/"slices", h)\
P_3 &= cal(S)(r, "slice" times (2pi)/"slices", 0)\
P_4 &= cal(S)(r, ("slice" + 1) times (2pi)/"slices", 0) 
$

Sendo $"slice"$ a iteração atual, $"slices"$ o número de _slices_ e $h$ a altura do cilindro.

Com a mesma ordem do cone, os triângulos são adicionados pela ordem: ${P_1, P_3, P_4} + {P_1, P_4, P_2}$.

Também são adicionados os triângulos das bases pela ordem: ${M, P_1, P_2}$ + ${M_"base", P_3, P_4}$ a cada iteração.

= _Parsing_ dos arquivos de cena `.xml` <parsing>

A _engine_ começa a ler o arquivo `.xml`, passado como parâmetro ao programa, fazendo recurso à biblioteca _TinyXML2_ @tinyxml2. A estrutura dos arquivos de cena `.xml` está de acordo com a que foi requerida pelo trabalho prático e os seus exemplos. Para já, só é configurável o tamanho da janela da _engine_, posição, olhar, vetor _up_, _FOV_ #footnote[_Field of View_], _z-near_ e _z-far_ da câmara e modelos dentro de grupos (podendo um grupo estar dentro de outro). 

O programa lê essas informações e coloca-as em memória para rápido acesso na renderização. Informações extra são simplesmente ignoradas. O programa só dá erro quando o formato dos parâmetros configuráveis não está correto.

== _Parsing_ dos Modelos

Aquando o _parser_ de arquivos de cena `.xml` encontra um elemento com o nome `<model>`, o ficheiro com nome do atributo `file` desse elemento é lido de acordo com a sua extensão, resultado num ```cpp Model```, uma estrutura de dados que guarda uma lista de vértices que será posteriormente renderizada.

=== Parsing de `.3d`

Caso o ficheiro encontrado na fase descrita no parágrafo anterior tenha como extensão `.3d` é chamado o _parser_ de modelos `.3d`. Esse _parser_ começa por ler a primeira linha #footnote[Recorde-se o exemplo do ficheiro: #link(<3d_file>)[Ficheiro exemplo retângulo]], e aloca um ```cpp std::vector``` com esse tamanho, $n$.

Após isso, são lidas $n$ linhas, que são transformadas em vértices e adicionadas ao _vector_ criado inicialmente pela ordem de leitura.

No fim, é criado a estrutura de dados ```cpp Model``` com a lista desses vértices, dentro do `World`, que contém os dados para a _engine_ renderizar o mundo.

#import "@preview/showybox:2.0.1": showybox

#showybox(
  frame: (
    border-color: red.darken(50%),
    title-color: red.lighten(60%),
    body-color: red.lighten(80%)
  ),
  title-style: (
    color: black,
    weight: "regular",
    align: center
  ),
  shadow: (
    offset: 3pt,
  ),
  title: [*Atenção a ter para as próximas fases*],
  [Neste formato não há informação sobre índices de vértices, havendo vértices repetidos. Para a implementação de *_VBO with indexing_* será necessário a alteração deste formato para guardar tal. Nas fases futuras também será necessário guardar informação relativa a vetores normais e posições de texturas. Estes são aspetos que o grupo teve em consideração mas optou por escolher um formato mais simples de acordo com os requisitos para esta fase.]
)

=== Parsing de `.obj`

Como forma de expansão do projeto, para além do nosso formato `.3d`, também suportamos a leitura de `Wavefront .obj files`. 

O formato está especificado aqui: #link("https://en.wikipedia.org/wiki/Wavefront_.obj_file"). 

Este formato de ficheiro já tem suporte a indexação de vértices, posições de texturas e normais, pelo que, poderá ser um formato a considerar para se tornar o _standard_ do projeto, nas fases futuras.

Para já, da parte do nosso programa, o ficheiro `.obj` é carregado de acordo com as faces nele registadas e os seus vértices correspondentes, gerando, novamente o ```cpp std::vector``` de vértices que o ```cpp Model``` necessita.

Com isto, devido à existência de vários programas de modelação com suporte a exportação para este formato, tornámos a nossa _engine_ muito mais versátil, dando suporte a carregar modelos de terceiros.

#figure(image("fase1/bunny.png", width: 70%), caption: [Renderização de um #link("https://en.wikipedia.org/wiki/Stanford_bunny")[Stanford bunny]
 de um modelo `.obj`])

= _Build_ do Projeto

De forma a assegurar grande compatibilidade entre os vários sistemas operativos utilizados, recorremos ao _vcpkg_ @vcpkg.
Com o _CMake_ @cmake, o _vcpkg_ é responsável por assegurar o _setup_ das várias dependências nos vários sistemas operativos, cumprindo a necessidade de cada um tem para cada dependência. 

Com compatibilidade ótima do _vcpkg_ sobre as dependências usadas #footnote[GLFW @glfw, OpenGL @opengl, TinyXML2 @tinyxml2, ImGui @imgui, Glew @glew.], o projeto compila sem qualquer problema sem precisar de _sources_ ou _DLLs_ dessas dependências no nosso código fonte.

O resultado da execução do _CMake_ são dois executáveis: o gerador de primitivas e o programa da _engine_.

== GitHub Actions

Para concretizar o objetivo citado nos parágrafos anteriores, estando o projeto hospedado num repositório privado do _GitHub_ @github, demos uso à funcionalidade de _GitHub Actions_ @github_actions para, a cada _commit_ no repositório, testar a compilação nos sistemas operativos _Windows_, _MacOS_ e _Linux (Ubuntu)_. 

Com isto, a integração continua do projeto é mantida muito facilmente devido à facilidade do isolamento de pedaços de código de um _commit_ que deram problemas num compilador de um sistema operativo diferente.

Esta funcionalidade já foi extremamente útil nesta primeira fase do projeto, com a deteção de algumas diferenças de _headers_ nos diferentes compiladores.

#figure(image("fase1/github_actions_result.png"), caption: [Resultado de um _commit_ após serem rodadas todas as verificações da _Action_])

Futuramente, também temos ideia em implementar alguns testes unitários, fazendo uso ainda mais desta ferramenta.

= A _Engine_

De uma forma geral, a _engine_ é o programa responsável por ler o ficheiro de cena `.xml` e renderizar o mundo descrito nesse ficheiro.
A mesma é simplesmente um motor de renderização 3D, que além de desenhar os modelos, tem também suporte a interações com o utilizador, como alterar a posição da câmara, ativar/desativar funcionalidades do _OpenGL_ e mostrar informações relevantes do mundo.

Essencialmente, o programa da _engine_ começa por criar uma classe `World` a partir do ficheiro de cena `.xml` passado como argumento. Esta classe contém informações tais como o nome do mundo, informações da janela em que o programa está a correr, a câmara e os modelos em questão. Seguidamente, é criada a classe `Engine`, que implementa funcionalidades pretendidas do OpenGL. Esta contém métodos para inicializar a _engine_ propriamente dita, renderizar o mundo, alterar definições do mundo e processar _input_ do utilizador.

O ciclo de vida da _engine_ pode ser descrito através do seguinte bloco de código:

#align(center, ```cpp
void Engine::Run() {
  Init();
  
  while (!WindowShouldClose(...)) {
    PollEvents();
    ProcessInput();
    Render();
  }
  
  Shutdown();
}
```)

A função `Init` é responsável por inicializar a _engine_ e todo o contexto do OpenGL.

A função `WindowShouldClose` #footnote[A função é referente à biblioteca _GLFW_.] <ref_glfw_footnote> tem como objetivo verificar se o utilizador fechou a janela. Em caso positivo, o ciclo é terminado e a _engine_ é desligada.

Dentro do corpo do ciclo, a função `PollEvents` @ref_glfw_footnote é responsável por processar todos os eventos que ocorreram desde a última iteração do ciclo; a função `ProcessInput` é responsável por processar, desses eventos, os de _input_ do utilizador, como por exemplo, teclas que  na posição da câmara e a função `Render` é responsável por desenhar o mundo no ecrã em conformidade.

Muito importante é a função `Render`. Esta, itera sobre todos os modelos do mundo e desenha-os no ecrã, aplicando a função `glVertex3f` a cada um dos vértices #footnote[Sempre com `glBegin(GL_TRIANGLES)` e `glEnd` antes e depois, respetivamente.].

== _GLFW_

Para gestão de janelas, eventos e contextos OpenGL, utilizamos a biblioteca _GLFW_ @glfw. Esta biblioteca, para além da sua utilização ser muito simples e intuitiva (em relação ao _GLUT_), tem suporte a algumas funcionalidades que demos uso. Algumas destas funcionalidades como suporte a alternação ligar e desligar _VSync_ em _runtime_, suporte a MSSA, foram implementadas, e serão explicitadas num próximo capítulo.

== _GLEW_

Para carregar as funções de OpenGL, também utilizamos a biblioteca _GLEW_. Esta biblioteca é necessária para carregar as funções de OpenGL de versões mais recentes, que não são carregadas por defeito por alguns sistemas operativos.

== Interações

O ponto mais diferencial da nossa _engine_ é a sua interação com o utilizador através de uma interface gráfica e controlo de rato e teclado. 

De modo intuitivo, o utilizador pode alterar a posição da câmara de forma a rodar em torno do ponto para onde a câmara está a olhar, clicando e arrastanto o rato. O _scroll_ do rato também é suportado, e altera a distância da câmara ao ponto para onde está a olhar. Estas alterações são feitas em tempo real numa movimentação suave e contínua #footnote[Não é de facto contínua, mas as alterações entre _frames_ são tão pequenas que dá a sensação de ser contínua.].

A implementação desta funcionalidade segue os princípios de coordenadas esféricas, que as coordenadas cartesianas da câmara são convertidas para coordenadas esféricas, os ângulos (ou raio no caso do _scroll_) são alterados conforme a interação do utilizador, e as coordenadas esféricas são convertidas de volta para coordenadas cartesianas, atualizando a posição da câmara.

```cpp
void Camera::ProcessInput(const float x_offset, const float y_offset, const float scroll_offset)
{
    float radius, alpha, beta;
    (position - looking_at).ToSpherical(&radius, &alpha, &beta);

    alpha -= x_offset * sensitivity;
    beta -= y_offset * sensitivity;
    radius -= scroll_offset * scroll_sensitivity;

    // Verificação de limites...

    const auto after = Vec3fSpherical(radius, alpha, beta);
    position = after + looking_at;
}
```

=== _ImGui_

A parte mais interessante está de facto dentro do menu do _ImGui_.

#figure(image("fase1/janela principal.png"), caption: [Vista principal do menu da interface])

Nesta janela, podendo ser arrastada para qualquer lugar dentro do programa, numa forma de menus colapsados, temos a opção de visualizar opções do mundo, _settings_ do programa e _FPS_ #footnote[_Frames per Second_, dados pelo _ImGui_ que usa o número de chamadas ao _render_ da sua janela para calculá-los.].

#figure(image("fase1/janela settings.png", width: 90%), caption: [Vista _ImGui_ com menu de _Settings_ aberto])

Abrindo a aba das _Settings_, é possível habilitar ou desabilitar funcionalidades como:
- *_VSync_*, por parte do _GLFW_, que ativa/desativa a sincronização vertical;
- *_Cull Faces_*, por parte do _OpenGL_, que mostra/esconde faces com normal não direcionadas para a câmara;
- *_Wireframe_*, por parte do _OpenGL_, que alterna entre mostrar uma estrutura de arame entre os triângulos ou preenchê-los com cor completamente;
- *_Render Axis_*, por parte da _engine_ com _OpenGL_, que mostra/esconde os eixos do gráfico;
- *_MSSA_*, por parte do _OpenGL_ + _GLFW_, que liga ou desliga o MSSA #footnote[_Multisample Anti-Aliasing_].

A nossa implementação do _MSSA_, toma vantagem de não termos implementado o nosso próprio _framebuffer_, e faz uso de um _buffer_ _multisample_, que os sistemas conseguem fornecer na criação de janelas.

Abrindo a aba _Environment_ dentro das _Settings_, informações do sistema operativo e versões de bibliotecas são mostradas.

#figure(image("fase1/janela world.png", width: 90%), caption: [Vista _ImGui_ com menu de _World_ aberto])

Passando para a aba _World_ é possível ver informações do  tamanho da janela da _engine_ (a janela é redimensionável) e é possível alterar os parâmetros da câmara durante a execução do programa, arrastando como um slider ou clicando duas vezes com o rato e escrever o valor que queremos. Também é possível restabelecer a posição inicial da câmara clicando no botão _Reset_.

Abrindo a aba _World_, temos informação do mundo carregado a partir da cena `.xml`.

#figure(image("fase1/janela vertices.png", width: 90%), caption: [Vista _ImGui_ com menu de _World Group_ aberto])

Nesta aba é possível ver informações de todos os modelos, como as posições dos seus vértices (a única informação neles presente até agora). Futuramente queremos expandir esta funcionalidade para, por exemplo, carregar novos modelos durante a execução, editá-los e ou removê-los.

= Conclusão

Para concluir, o grupo está muito satisfeito com o resultado final desta primeira etapa do projeto. 
Acreditamos ter uma base muito sólida para realizar as próximas etapas, também, com sucesso.

#bibliography("bibliography.bib", full: true)
