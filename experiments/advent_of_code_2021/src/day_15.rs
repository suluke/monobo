use super::get_input;
fn day() -> &'static str {
    let m = module_path!();
    &m[m.len() - 2..]
}

struct Grid {
    fields: Vec<Vec<usize>>,
}
impl Grid {
    fn parse(input: &str) -> Self {
        let w = input.lines().next().unwrap().trim().len() + 2; // sentinels
        let mut fields = Vec::<Vec<usize>>::new();
        let mut sentinel = Vec::with_capacity(w);
        sentinel.resize(w, usize::MAX);
        fields.push(sentinel);
        for line in input.lines() {
            let f_line: Vec<_> = Some(usize::MAX)
                .iter()
                .copied()
                .chain(line.trim().chars().map(|c| (c as u8 - '0' as u8) as usize))
                .chain(Some(usize::MAX))
                .collect();
            assert_eq!(fields[0].len(), f_line.len());
            fields.push(f_line);
        }
        let mut sentinel = Vec::with_capacity(w);
        sentinel.resize(w, usize::MAX);
        fields.push(sentinel);
        Self { fields }
    }
    fn new(w: usize, h: usize) -> Self {
        let mut fields = Vec::new();
        for _ in 0..h {
            let mut row = Vec::new();
            row.resize(w, usize::MAX);
            fields.push(row);
        }
        Self {fields}
    }
    fn get(&self, pos: (usize, usize)) -> usize {
        let (x, y) = pos;
        self.fields[y][x]
    }
    fn set(&mut self, pos: (usize, usize), val: usize) {
        let (x, y) = pos;
        self.fields[y][x] = val;
    }
    fn width(&self) -> usize {
        self.fields.first().unwrap().len()
    }
    fn height(&self) -> usize {
        self.fields.len()
    }
}

#[derive(Clone)]
struct Path {
    head: (usize, usize),
    weight: usize,
}
impl Path {
    fn new() -> Self {
        Self { head: (1,1), weight: 0 }
    }
    fn head(&self) -> (usize, usize) {
        self.head
    }
    fn extend(&self, next: (usize, usize), weight: usize) -> Self {
        let mut extended = self.clone();
        extended.head = next;
        extended.weight = extended.weight.saturating_add(weight);
        extended
    }
}

struct PathSearcher<'a> {
    grid: &'a Grid,
    weights: Grid,
    lowest: usize,
    unfinished: Vec<Path>,
}
impl<'a> PathSearcher<'a> {
    fn new(grid: &'a Grid) -> Self {
        let mut unfinished = Vec::new();
        unfinished.push(Path::new());
        let mut weights = Grid::new(grid.width(), grid.height());
        weights.set((0, 0), 0);
        Self {
            grid,
            weights,
            lowest: usize::MAX - 1,
            unfinished,
        }
    }
    fn advance_back(&mut self) {
        // make sure we have right or down be the last added direction since we want `lowest` to be initialized to something sensible as early as possoble
        if let Some(path) = self.unfinished.pop() {
            let pos = path.head();
            let total = path.weight;
            // println!(
            //     "Advance {:?} with weight {}, known lowest being {}",
            //     pos, total, self.lowest
            // );
            if total < self.lowest {
                for dy in -1..=1 {
                    for dx in -1..=1 {
                        if dx == 0 || dy == 0 && !(dx == 0 && dy == 0) {
                            let newpos = (
                                (pos.0 as isize + dx) as usize,
                                (pos.1 as isize + dy) as usize,
                            );
                            let new_weight = self.grid.get(newpos);
                            let new_total = total.saturating_add(new_weight);
                            if new_total < self.lowest {
                                if newpos.1 == self.grid.height() - 2
                                    && newpos.0 == self.grid.width() - 2
                                {
                                    // reached end
                                    self.weights.set(newpos, new_total);
                                    self.lowest = new_total;
                                } else if new_total < self.weights.get(newpos) /*&& !path.points.contains(&newpos)*/ {
                                    self.weights.set(newpos, new_total);
                                    self.unfinished.push(path.extend(newpos, new_weight));
                                }
                            }
                        }
                    }
                }
            } // else die
        }
    }
    fn find_lowest(mut self) -> usize {
        while !self.unfinished.is_empty() {
            self.advance_back();
        }
        self.lowest
    }
}

fn part1() -> std::io::Result<()> {
    let input = get_input(day());
    let grid = Grid::parse(&input);
    let searcher = PathSearcher::new(&grid);
    let res = searcher.find_lowest();
    println!("day{}/pt1: {}", day(), res);
    Ok(())
}

fn expand(input: &str) -> String {
    let mut fullwidth = String::new();
    let incchar = |c, i| ((c as u8 - '1' as u8 + i) % 9 + '1' as u8) as char;
    for line in input.lines() {
        for i in 0..5 {
            for c in line.trim().chars() {
                let newc = incchar(c, i);
                fullwidth.push(newc);
            }
        }
        fullwidth.push('\n');
    }
    let mut res = fullwidth.clone();
    for i in 1..5 {
        for line in fullwidth.lines() {
            for c in line.trim().chars() {
                let newc = incchar(c, i);
                res.push(newc);
            }
            res.push('\n');
        }
    }
    res
}

fn part2() -> std::io::Result<()> {
    let input = expand(&get_input(day()));
    let grid = Grid::parse(&input);
    let searcher = PathSearcher::new(&grid);
    let res = searcher.find_lowest();
    println!("day{}/pt2: {}", day(), res);
    Ok(())
}
#[allow(unused)]
pub fn solve() -> std::io::Result<()> {
    part1()?;
    part2()
}
#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test() {
        let input = "1163751742\n\
        1381373672\n\
        2136511328\n\
        3694931569\n\
        7463417111\n\
        1319128137\n\
        1359912421\n\
        3125421639\n\
        1293138521\n\
        2311944581";
        let grid = Grid::parse(&input);
        let searcher = PathSearcher::new(&grid);
        assert_eq!(40, searcher.find_lowest());
    }
    #[test]
    fn test2() {
        let input = "9";
        assert_eq!(&expand(input), "91234\n12345\n23456\n34567\n45678\n");
    }
}
