#let project(title: "", authors: (), date: none, group: [], body) = {
  set document(author: authors.map(a => a.name), title: title)
  set page(numbering: "1", number-align: center)
  set text(font: "New Computer Modern", lang: "pt", region: "pt", size: 11pt)
  show math.equation: set text(weight: 400)

  show par: set block(above: 0.75em, below: 1.5em)

  set par(leading: 0.58em)

  show heading: set block(above: 2em)

  show figure: it => {
    align(center, block(below: 1.5em, it))
  }

  set heading(numbering: "1.")

  show link: underline

  // Display inline code in a small box
  // that retains the correct baseline.
  show raw.where(block: false): box.with(
    fill: luma(240),
    inset: (x: 3pt, y: 0pt),
    outset: (y: 3pt),
    radius: 2pt,
  )
  
  // Display block code in a larger block
  // with more padding.
  show raw.where(block: true): block.with(
    breakable: false,
    fill: luma(240),
    inset: 10pt,
    radius: 4pt,
  )
  
  align(center)[
    #block(text(weight: 700, 1.75em, 
    title))
    #date
    #v(1.5em, weak: true)
    #text(1.2em, group)
  ]
  pad(
    top: -0.5em,
    bottom: 0.3em,
    x: 1em,
    grid(
      columns: (1fr,) * 4,
      gutter: 1em,
      ..authors.map(author => align(center)[
        *#author.name* \
        #author.number
      ]),
    ),
  )

  set par(justify: true)

  body
}