use super::get_input;
fn day() -> &'static str {
    let m = module_path!();
    &m[m.len() - 2..]
}

#[derive(Debug)]
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
    fn step(&mut self) -> usize {
        for field in self.fields.iter_mut().flatten() {
            *field = field.saturating_add(1);
        }
        let mut worklist: Vec<(usize, usize)> = self
            .fields
            .iter()
            .flatten()
            .enumerate()
            .filter(|(_, &val)| val > 9 && val != usize::MAX)
            .map(|(pos, _)| (pos % self.fields.len(), pos / self.fields.len()))
            .collect();
        while let Some((x, y)) = worklist.pop() {
            if self.fields[y][x] == usize::MAX {
                continue;
            }
            self.fields[y][x] = usize::MAX;
            for dy in -1..=1 {
                for dx in -1..=1 {
                    let (nx, ny) = (((x as isize) + dx) as usize, ((y as isize) + dy) as usize);
                    let n = &mut self.fields[ny][nx];
                    *n = (*n).saturating_add(1);
                    if *n > 9 && *n != usize::MAX {
                        worklist.push((nx, ny));
                    }
                }
            }
        }
        let mut sum = 0;
        for y in 1..(self.fields.len() - 1) {
            for x in 1..(self.fields[y].len() - 1) {
                if self.fields[y][x] == usize::MAX {
                    self.fields[y][x] = 0;
                    sum += 1;
                }
            }
        }
        sum
    }
}

fn part1() -> std::io::Result<()> {
    let input = get_input(day());

    let mut grid = Grid::parse(&input);
    let mut sum = 0;
    for _ in 0..100 {
        sum += grid.step();
    }
    let res = sum;
    println!("day{}/pt1: {}", day(), res);
    Ok(())
}
fn part2() -> std::io::Result<()> {
    let input = get_input(day());

    let mut grid = Grid::parse(&input);
    let mut i = 1;
    while grid.step() != 100 {
        i += 1;
    }

    println!("day{}/pt2: {}", day(), i);
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
        let input = "11111\n\
        19991\n\
        19191\n\
        19991\n\
        11111";
        let mut grid = Grid::parse(&input);
        assert_eq!(grid.step(), 9);
    }
}
