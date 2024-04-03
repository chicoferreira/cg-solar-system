#let ratio = 0.5 / calc.sqrt(2)

#let project(x, y, z, size) = {
  let r = size / 2
  (
    x * r - z * r * ratio + r, 
    -y * r + z * r * ratio + r
  )
}

#let spherical_to_cartesian(radius, alpha, beta) = (
  radius * calc.sin(alpha) * calc.cos(beta),
  radius * calc.sin(beta),
  radius * calc.cos(alpha) * calc.cos(beta),
)

#let polar_to_cartesian(radius, alpha, y) = (
  radius * calc.sin(alpha),
  y,
  radius * calc.cos(alpha)
)

#let to_pt(content) = {
  if type(content) == "array" {
    content.map(v => v * 1pt)
  } else {
    content * 1pt
  }
}

#let psize = 3pt
#let point(x, y, fill: black) = {
  place(
    dx: x - psize/2, dy: y - psize/2, 
    circle(radius: psize/2, fill: fill)
  )
} 

#let draw_axis() = {
    place(line(stroke: red, start: origin, end: project(1,0,0, size)))
    place(line(stroke: green, start: origin, end: project(0,1,0, size)))
    place(line(stroke: blue, start: origin, end: project(0,0,1, size)))
}

#let sub_vector(v1, v2) = v1.zip(v2).map(v => v.at(0) - v.at(1))

#let is_triangle_ccw(p1, p2, p3) = {
  let edgeA = sub_vector(p2, p1)
  let edgeB = sub_vector(p3, p1)

  return (edgeA.at(0) * edgeB.at(1) - edgeA.at(1) * edgeB.at(0)) < 0
}
