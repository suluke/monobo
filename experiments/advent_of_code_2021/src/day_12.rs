use super::get_input;
fn day() -> &'static str {
    let m = module_path!();
    &m[m.len() - 2..]
}

use std::cell::RefCell;
use std::collections::HashMap;
use std::fmt::{Debug, Formatter};
use std::rc::Rc;

#[derive(Clone, Eq, PartialEq)]
struct Node {
    name: String,
    adjacent: RefCell<Vec<Rc<Node>>>,
}
impl Debug for Node {
    fn fmt(&self, f: &mut Formatter) -> Result<(), std::fmt::Error> {
        f.write_str(&format!("\"{}\"", self.name))
    }
}
impl std::hash::Hash for Node {
    fn hash<H>(&self, hasher: &mut H)
    where
        H: std::hash::Hasher,
    {
        std::ptr::hash(self, hasher);
    }
}
impl Node {
    fn new(name: &str) -> Self {
        Self {
            name: name.to_owned(),
            adjacent: RefCell::new(Vec::new()),
        }
    }
    fn is_big(&self) -> bool {
        return self.name.chars().next().unwrap().is_uppercase();
    }
    fn parse(input: &str) -> Rc<Node> {
        let start = Rc::new(Node::new("start"));
        let mut nodes: HashMap<String, Rc<Node>> = HashMap::new();
        nodes.insert(start.name.clone(), start.clone());
        for line in input.lines() {
            let (l, r) = line.split_once("-").unwrap();
            let l = nodes
                .entry(l.to_owned())
                .or_insert_with(|| Rc::new(Node::new(l)))
                .clone();
            let r = nodes
                .entry(r.to_owned())
                .or_insert_with(|| Rc::new(Node::new(r)))
                .clone();
            l.adjacent.borrow_mut().push(r.clone());
            r.adjacent.borrow_mut().push(l.clone());
        }
        start
    }
}

struct Path {
    nodes: Vec<Rc<Node>>,
}
impl Path {
    fn new(start: Rc<Node>) -> Self {
        Self { nodes: vec![start] }
    }
    fn ended(&self) -> bool {
        let head = self.nodes.last().unwrap();
        head.name == "end"
    }
    fn extend(&self, with: Rc<Node>) -> Self {
        let mut nodes = self.nodes.clone();
        nodes.push(with);
        Self { nodes }
    }
    fn successors(self) -> Vec<Path> {
        let head = self.nodes.last().unwrap();
        if self.ended() {
            vec![self]
        } else {
            head.adjacent
                .borrow()
                .iter()
                .filter(|&n| n.is_big() || !self.nodes.contains(n))
                .map(|n| self.extend(n.clone()))
                .collect()
        }
    }
    fn successors2(self) -> Vec<Path> {
        let head = self.nodes.last().unwrap();
        if self.ended() {
            vec![self]
        } else {
            let small: Vec<Rc<Node>> = self.nodes.iter().filter(|n| !n.is_big()).cloned().collect();
            let unique: std::collections::HashSet<_> = small.iter().cloned().collect();
            let has_dbl = small.len() > unique.len();
            head.adjacent
                .borrow()
                .iter()
                .filter(|&n| {
                    n.is_big()
                        || !self.nodes.contains(n)
                        || (n.name != "start" && n.name != "end" && !has_dbl)
                })
                .map(|n| self.extend(n.clone()))
                .collect()
        }
    }
}
impl Debug for Path {
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        self.nodes.fmt(f)
    }
}

fn part1() -> std::io::Result<()> {
    let input = get_input(day());

    let mut paths = vec![Path::new(Node::parse(&input))];
    while paths.iter().any(|p| !p.ended()) {
        paths = paths.drain(0..).map(Path::successors).flatten().collect();
    }

    println!("day{}/pt1: {}", day(), paths.len());
    Ok(())
}
fn part2() -> std::io::Result<()> {
    let input = get_input(day());

    let mut paths = vec![Path::new(Node::parse(&input))];
    while paths.iter().any(|p| !p.ended()) {
        paths = paths.drain(0..).map(Path::successors2).flatten().collect();
    }

    println!("day{}/pt2: {}", day(), paths.len());
    Ok(())
}
pub fn solve() -> std::io::Result<()> {
    part1()?;
    part2()
}
#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn test() {
        let input = "start-A\n\
        start-b\n\
        A-c\n\
        A-b\n\
        b-d\n\
        A-end\n\
        b-end";
        let mut paths = vec![Path::new(Node::parse(&input))];
        while paths.iter().any(|p| !p.ended()) {
            paths = paths.drain(0..).map(Path::successors).flatten().collect();
        }
        assert_eq!(10, paths.len());
    }
    #[test]
    fn test2() {
        let input = "start-A\n\
        start-b\n\
        A-c\n\
        A-b\n\
        b-d\n\
        A-end\n\
        b-end";
        let mut paths = vec![Path::new(Node::parse(&input))];
        while paths.iter().any(|p| !p.ended()) {
            paths = paths.drain(0..).map(Path::successors2).flatten().collect();
        }
        assert_eq!(36, paths.len());
    }
}
