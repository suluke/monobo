#[derive(Debug)]
struct Line {
    p1: (isize, isize),
    p2: (isize, isize),
}
impl Line {
    fn is_axis_aligned(&self) -> bool {
        self.p1.0 == self.p2.0 || self.p1.1 == self.p2.1
    }
    fn is_diag(&self) -> bool {
        let grad = self.gradient();
        grad.0 != 0 && grad.1 != 0
    }
    fn is_aa_or_diag(&self) -> bool {
        self.is_axis_aligned() || self.is_diag()
    }
    fn parse(line: &str) -> Line {
        let mut nums = line
            .split(" -> ")
            .map(|pt| pt.split(","))
            .flatten()
            .map(|val| val.parse::<isize>().unwrap());
        let p1 = (nums.next().unwrap(), nums.next().unwrap());
        let p2 = (nums.next().unwrap(), nums.next().unwrap());
        Line { p1, p2 }
    }
    fn gradient(&self) -> (isize, isize) {
        let mut d = (self.p2.0 - self.p1.0, self.p2.1 - self.p1.1);
        let dx_abs = isize::abs(d.0);
        let dy_abs = isize::abs(d.1);
        if d.0 == 0 && d.1 == 0 {
            (0, 0)
        } else if d.0 == 0 || d.1 == 0 {
            d.0 /= dx_abs + dy_abs;
            d.1 /= dx_abs + dy_abs;
            assert!(d.0 != 0 || d.1 != 0);
            d
        } else if dx_abs == dy_abs {
            d.0 /= dx_abs;
            d.1 /= dx_abs;
            d
        } else {
            panic!("Unsupported line type");
        }
    }
}

struct Field {
    width: usize,
    _height: usize,
    cells: Vec<isize>,
}
impl Field {
    fn new(_xmin: isize, xmax: isize, _ymin: isize, ymax: isize) -> Self {
        let mut cells = Vec::<isize>::new();
        let width = (xmax + 1) as usize;
        let height = (ymax + 1) as usize;
        cells.resize((width * height) as usize, 0);
        // println!("field with: {}, height: {}", width, height);
        Self {
            width,
            _height: height,
            cells,
        }
    }
    fn plot(&mut self, line: &Line) {
        let mut pos = (line.p1.0, line.p1.1);
        let grad = line.gradient();
        loop {
            // println!("line: {:?}, grad: {:?}, pos: {:?}", line, grad, pos);
            self.cover(pos);
            if pos == line.p2 {
                break;
            }
            pos = (pos.0 + grad.0, pos.1 + grad.1);
        }
    }
    fn cover(&mut self, cell: (isize, isize)) {
        // println!("{:?}", cell);
        self.cells[(cell.0 + cell.1 * self.width as isize) as usize] += 1;
    }
    fn count_multi_covered(&self) -> isize {
        self.cells.iter().copied().filter(|cell| *cell > 1).count() as isize
    }
}

fn parse_lines(input: &str) -> Vec<Line> {
    input.lines().map(Line::parse).collect()
}

macro_rules! max {
    ($x:expr) => ( $x );
    ($x:expr, $($xs:expr),+) => {
        {
            use std::cmp::max;
            max($x, max!( $($xs),+ ))
        }
    };
}

macro_rules! min {
    ($x:expr) => ( $x );
    ($x:expr, $($xs:expr),+) => {
        {
            use std::cmp::min;
            min($x, min!( $($xs),+ ))
        }
    };
}

fn part1() -> std::io::Result<()> {
    let mut input_path = std::path::PathBuf::new();
    input_path.push(env!("CARGO_MANIFEST_DIR"));
    input_path.push("input");
    input_path.push("05.txt");
    let input = std::fs::read_to_string(input_path).unwrap();
    let mut lines = parse_lines(&input);
    let aaLines: Vec<Line> = lines.drain(0..).filter(Line::is_axis_aligned).collect();
    let (xmin, xmax, ymin, ymax) = aaLines.iter().fold(
        (isize::MAX, isize::MIN, isize::MAX, isize::MIN),
        |acc, line| {
            (
                min!(acc.0, line.p1.0, line.p2.0),
                max!(acc.1, line.p1.0, line.p2.0),
                min!(acc.2, line.p1.1, line.p2.1),
                max!(acc.3, line.p1.1, line.p2.1),
            )
        },
    );
    let mut field = Field::new(xmin, xmax, ymin, ymax);
    for line in &aaLines {
        field.plot(line);
    }
    println!("day5/pt1: {}", field.count_multi_covered());
    Ok(())
}

fn part2() -> std::io::Result<()> {
    let mut input_path = std::path::PathBuf::new();
    input_path.push(env!("CARGO_MANIFEST_DIR"));
    input_path.push("input");
    input_path.push("05.txt");
    let input = std::fs::read_to_string(input_path).unwrap();
    let mut lines = parse_lines(&input);
    // is_aa_or_diag is the ONLY difference to part1
    let aaLines: Vec<Line> = lines.drain(0..).filter(Line::is_aa_or_diag).collect();
    let (xmin, xmax, ymin, ymax) = aaLines.iter().fold(
        (isize::MAX, isize::MIN, isize::MAX, isize::MIN),
        |acc, line| {
            (
                min!(acc.0, line.p1.0, line.p2.0),
                max!(acc.1, line.p1.0, line.p2.0),
                min!(acc.2, line.p1.1, line.p2.1),
                max!(acc.3, line.p1.1, line.p2.1),
            )
        },
    );
    let mut field = Field::new(xmin, xmax, ymin, ymax);
    for line in &aaLines {
        field.plot(line);
    }
    println!("day5/pt1: {}", field.count_multi_covered());
    Ok(())
}

pub fn solve() -> std::io::Result<()> {
    part1()?;
    part2()
}
