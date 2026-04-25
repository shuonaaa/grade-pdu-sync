import os
import random


COURSES = [
    # ── 公共必修课 101-130 ───────────────────────────────
    ('高等数学A',                   4, 64,  'percentile'),
    ('高等数学B',                   3, 48,  'percentile'),
    ('高等数学C',                   2, 32,  'percentile'),
    ('线性代数',                    3, 48,  'percentile'),
    ('概率论与数理统计',             3, 48,  'percentile'),
    ('大学物理A',                   4, 64,  'percentile'),
    ('大学物理B',                   3, 48,  'percentile'),
    ('大学物理实验',                 1, 32,  'examCheck'),
    ('大学英语A',                   3, 48,  'percentile'),
    ('大学英语B',                   3, 48,  'percentile'),
    ('大学英语C',                   2, 32,  'percentile'),
    ('大学英语写作',                 2, 32,  'percentile'),
    ('体育A',                       1, 32,  'examCheck'),
    ('体育B',                       1, 32,  'examCheck'),
    ('体育C',                       1, 32,  'examCheck'),
    ('体育D',                       1, 32,  'examCheck'),
    ('思想道德与法治',               3, 48,  'examCheck'),
    ('中国近现代史纲要',             2, 32,  'examCheck'),
    ('马克思主义基本原理',           3, 48,  'examCheck'),
    ('毛泽东思想和中国特色社会主义理论体系概论', 3, 48, 'examCheck'),
    ('习近平新时代中国特色社会主义思想概论', 2, 32, 'examCheck'),
    ('形势与政策A',                  1, 16,  'examCheck'),
    ('形势与政策B',                  1, 16,  'examCheck'),
    ('大学语文',                    2, 32,  'examCheck'),
    ('军事理论',                    2, 36,  'examCheck'),
    ('职业规划与就业指导',           1, 16,  'examCheck'),
    ('创新创业基础',                 2, 32,  'examCheck'),
    ('心理健康教育',                 1, 16,  'examCheck'),
    ('劳动教育',                    1, 16,  'examCheck'),
    ('安全教育',                    1, 16,  'examCheck'),

    # ── 计算机类 131-175 ────────────────────────────────
    ('程序设计基础（C语言）',         4, 64,  'percentile'),
    ('程序设计基础（Python）',        3, 48,  'percentile'),
    ('数据结构',                    4, 64,  'percentile'),
    ('算法设计与分析',               3, 48,  'percentile'),
    ('计算机组成原理',               4, 64,  'percentile'),
    ('操作系统',                    4, 64,  'percentile'),
    ('计算机网络',                   4, 64,  'percentile'),
    ('数据库原理与应用',             3, 48,  'percentile'),
    ('软件工程',                    3, 48,  'percentile'),
    ('编译原理',                    3, 48,  'percentile'),
    ('人工智能导论',                 3, 48,  'percentile'),
    ('机器学习',                    3, 48,  'percentile'),
    ('深度学习',                    3, 48,  'percentile'),
    ('计算机视觉',                   3, 48,  'percentile'),
    ('自然语言处理',                 3, 48,  'percentile'),
    ('Web开发技术',                  3, 48,  'percentile'),
    ('移动应用开发',                 3, 48,  'percentile'),
    ('云计算与大数据',               3, 48,  'percentile'),
    ('信息安全基础',                 3, 48,  'percentile'),
    ('密码学',                      3, 48,  'percentile'),
    ('嵌入式系统',                   3, 48,  'percentile'),
    ('计算机图形学',                 3, 48,  'percentile'),
    ('数字图像处理',                 3, 48,  'percentile'),
    ('分布式系统',                   3, 48,  'percentile'),
    ('程序设计实验（C语言）',         1, 32,  'examCheck'),
    ('数据结构实验',                 1, 32,  'examCheck'),
    ('操作系统实验',                 1, 32,  'examCheck'),
    ('计算机网络实验',               1, 32,  'examCheck'),
    ('数据库实验',                   1, 32,  'examCheck'),
    ('软件工程实践',                 2, 32,  'examCheck'),
    ('毕业设计（论文）',             6, 0,   'fivePoint'),
    ('课程设计A',                   2, 32,  'fivePoint'),
    ('课程设计B',                   2, 32,  'fivePoint'),
    ('认识实习',                    1, 16,  'examCheck'),
    ('生产实习',                    2, 32,  'examCheck'),

    # ── 数学与统计类 176-195 ────────────────────────────
    ('数学分析',                    5, 80,  'percentile'),
    ('高等代数',                    4, 64,  'percentile'),
    ('解析几何',                    3, 48,  'percentile'),
    ('实变函数',                    3, 48,  'percentile'),
    ('复变函数',                    3, 48,  'percentile'),
    ('数值分析',                    3, 48,  'percentile'),
    ('运筹学',                      3, 48,  'percentile'),
    ('离散数学',                    4, 64,  'percentile'),
    ('组合数学',                    3, 48,  'percentile'),
    ('数理统计',                    3, 48,  'percentile'),
    ('随机过程',                    3, 48,  'percentile'),
    ('最优化方法',                   3, 48,  'percentile'),
    ('数学建模',                    3, 48,  'percentile'),
    ('图论',                        2, 32,  'percentile'),
    ('数学实验',                    2, 32,  'examCheck'),

    # ── 电子电气类 196-215 ─────────────────────────────
    ('电路分析',                    4, 64,  'percentile'),
    ('模拟电子技术',                 4, 64,  'percentile'),
    ('数字电子技术',                 4, 64,  'percentile'),
    ('信号与系统',                   4, 64,  'percentile'),
    ('自动控制原理',                 4, 64,  'percentile'),
    ('电机与拖动',                   3, 48,  'percentile'),
    ('电力电子技术',                 3, 48,  'percentile'),
    ('单片机原理与应用',             3, 48,  'percentile'),
    ('FPGA设计与应用',               3, 48,  'percentile'),
    ('传感器与检测技术',             3, 48,  'percentile'),
    ('电子线路实验',                 1, 32,  'examCheck'),
    ('数字电路实验',                 1, 32,  'examCheck'),
    ('控制系统实验',                 1, 32,  'examCheck'),
    ('PLC原理与应用',                3, 48,  'percentile'),
    ('工业互联网',                   3, 48,  'percentile'),

    # ── 机械类 216-235 ─────────────────────────────────
    ('工程力学',                    4, 64,  'percentile'),
    ('材料力学',                    4, 64,  'percentile'),
    ('机械原理',                    4, 64,  'percentile'),
    ('机械设计',                    4, 64,  'percentile'),
    ('制造工艺学',                   3, 48,  'percentile'),
    ('工程材料',                    3, 48,  'percentile'),
    ('热力学与流体力学',             3, 48,  'percentile'),
    ('工程图学',                    3, 48,  'percentile'),
    ('CAD/CAM技术',                  3, 48,  'percentile'),
    ('数控技术',                    3, 48,  'percentile'),
    ('机器人技术',                   3, 48,  'percentile'),
    ('工程测量实验',                 1, 32,  'examCheck'),
    ('机械设计实验',                 1, 32,  'examCheck'),
    ('金工实习',                    2, 32,  'examCheck'),
    ('机电综合实训',                 2, 32,  'examCheck'),

    # ── 经管类 236-260 ─────────────────────────────────
    ('微观经济学',                   3, 48,  'percentile'),
    ('宏观经济学',                   3, 48,  'percentile'),
    ('管理学原理',                   3, 48,  'percentile'),
    ('会计学原理',                   3, 48,  'percentile'),
    ('财务管理',                    3, 48,  'percentile'),
    ('市场营销',                    3, 48,  'percentile'),
    ('人力资源管理',                 3, 48,  'percentile'),
    ('统计学',                      3, 48,  'percentile'),
    ('计量经济学',                   3, 48,  'percentile'),
    ('国际贸易',                    3, 48,  'percentile'),
    ('项目管理',                    3, 48,  'percentile'),
    ('供应链管理',                   3, 48,  'percentile'),
    ('运营管理',                    3, 48,  'percentile'),
    ('商业数据分析',                 3, 48,  'percentile'),
    ('电子商务',                    3, 48,  'percentile'),

    # ── 人文社科类 261-300 ──────────────────────────────
    ('中国哲学史',                   2, 32,  'examCheck'),
    ('西方哲学史',                   2, 32,  'examCheck'),
    ('逻辑学',                      2, 32,  'percentile'),
    ('社会学概论',                   2, 32,  'examCheck'),
    ('法律基础',                    2, 32,  'examCheck'),
    ('中国文化概论',                 2, 32,  'examCheck'),
    ('世界文化概论',                 2, 32,  'examCheck'),
    ('文学欣赏',                    2, 32,  'examCheck'),
    ('艺术概论',                    2, 32,  'examCheck'),
    ('音乐鉴赏',                    1, 16,  'examCheck'),
    ('影视欣赏',                    1, 16,  'examCheck'),
    ('书法与篆刻',                   1, 16,  'examCheck'),
    ('摄影基础',                    1, 16,  'examCheck'),
    ('演讲与口才',                   1, 16,  'examCheck'),
    ('科技写作',                    2, 32,  'examCheck'),
    ('学术英语',                    2, 32,  'percentile'),
    ('日语入门',                    2, 32,  'fivePoint'),
    ('德语入门',                    2, 32,  'fivePoint'),
    ('韩语入门',                    2, 32,  'fivePoint'),
    ('传播学概论',                   2, 32,  'examCheck'),
]

OUT_PATH = os.path.join(os.path.dirname(__file__), '../database/Course_insert.sql')
OUT_PATH = os.path.normpath(OUT_PATH)

with open(OUT_PATH, 'w', encoding='utf-8') as f:
    f.write("-- 自动生成，勿手动编辑\n")
    f.write("USE week_7_server;\n\n")
    f.write("INSERT INTO Course (courseNumber, courseName, credit, courseHour, ScoreType) VALUES\n")

    used = set()
    def gen_course_num():
        while True:
            n = random.randint(10000000, 99999999)
            if n not in used:
                used.add(n)
                return n

    rows = []
    for name, credit, hour, stype in COURSES:
        num = gen_course_num()
        rows.append(f"    ({num}, '{name}', {credit}, {hour}, '{stype}')")

    f.write(",\n".join(rows))
    f.write(";\n")

print(f"共生成 {len(COURSES)} 门课程 -> {OUT_PATH}")
