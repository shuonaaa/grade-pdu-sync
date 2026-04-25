import random
import datetime
import os
import itertools as _it

TOTAL_TEACHERS = 1200

# 出生年份分布
BIRTH_DIST = [
    (range(1960, 1971), 0.20),
    (range(1971, 1981), 0.35),
    (range(1981, 1991), 0.35),
    (range(1991, 1997), 0.10),
]

SURNAMES = [
    '王','李','张','刘','陈','杨','赵','黄','周','吴',
    '徐','孙','马','朱','胡','郭','何','高','林','罗',
    '郑','梁','谢','宋','唐','许','邓','冯','韩','曹',
    '曾','彭','肖','田','董','袁','潘','于','蒋','蔡',
    '余','杜','叶','程','苏','魏','吕','丁','任','沈',
    '姚','卢','姜','崔','钟','谭','陆','汪','范','金',
    '石','廖','贾','夏','韦','付','方','白','邹','熊',
    '孟','秦','邱','侯','江','尹','薛','闫','段','雷',
    '黎','史','龙','陶','贺','顾','毛','郝','龚','邵',
    '万','钱','严','赖','覃','洪','武','莫','孔','汤',
]

# 60-80年代出生，命名风格较传统
_M1 = '建国军民志红胜明华强伟平振荣文光正海山春培德茂永广兴宝金玉福寿禄贵昌盛仁义礼智信忠孝廉耻维本祥瑞安康泰顺昭宪法道学诚信义宇乾坤运吉庆新'
_M2 = '国军民志强华明伟平荣文光正海山春培德茂永广兴宝金福寿禄贵昌盛仁义礼智信忠孝廉耻祥瑞安康泰顺昭宪法道学诚义乾坤运吉庆新辉斌彬勇刚毅峰涛磊'

_F1 = '秀淑美华英丽珍兰芳云红玉萍燕敏洁静慧娟梅桂凤素菊莲霞春青翠彩彩彩雪月霜蕾花香艳彩虹莺鸣晓晨曦璇琴琴书画诗文雅致慕容'
_F2 = '英华珍兰芳云玉萍燕敏洁静慧娟梅桂凤素菊莲霞春青翠彩雪月霜花香艳虹莺晓晨曦琴书画诗文雅秀淑美丽红蕾彩彩彩彩彩彩彩彩彩彩'

def _gen_double(c1, c2, n):
    pool = list({a + b for a, b in _it.product(c1, c2) if a != b})
    random.shuffle(pool)
    return pool[:n]

_M_SINGLE = list('建国军志明华强伟平荣文光正海山春德永广兴福寿贵昌盛仁义礼智忠孝祥瑞安康泰顺学诚勇刚毅峰涛磊')
_F_SINGLE = list('秀淑美华英丽珍兰芳云红玉萍燕敏洁静慧娟梅桂凤素菊莲霞春青翠雪月花香艳')

_MALE_DOUBLE   = _gen_double(_M1, _M2, 1000)
_FEMALE_DOUBLE = _gen_double(_F1, _F2, 1000)

def make_name(gender):
    surname = random.choice(SURNAMES)
    r = random.random()
    if r < 0.30:   # 老一辈单字名比例更高
        first = random.choice(_M_SINGLE if gender == '男' else _F_SINGLE)
    else:
        first = random.choice(_MALE_DOUBLE if gender == '男' else _FEMALE_DOUBLE)
    return surname + first

def make_gender():
    return '男' if random.random() < 0.6 else '女'

def random_birth_year():
    r = random.random()
    cumul = 0.0
    for year_range, weight in BIRTH_DIST:
        cumul += weight
        if r < cumul:
            return random.choice(list(year_range))
    return 1980

def random_birth_date(year):
    start = datetime.date(year, 1, 1)
    return start + datetime.timedelta(days=random.randint(0, 364))

def date_to_tid(d):
    return int(f"{d.year:04d}{d.month:02d}{d.day:02d}")

OUT_PATH = os.path.join(os.path.dirname(__file__), '../database/Teacher_insert.sql')
OUT_PATH = os.path.normpath(OUT_PATH)

BATCH = 200

used_tids = set()
rows = []

for _ in range(TOTAL_TEACHERS):
    gender = make_gender()
    name   = make_name(gender)

    # 生成不重复的 tid
    year = random_birth_year()
    date = random_birth_date(year)
    tid  = date_to_tid(date)
    # 同生日冲突时顺延1天直到唯一
    while tid in used_tids:
        date += datetime.timedelta(days=1)
        tid   = date_to_tid(date)
    used_tids.add(tid)

    rows.append((tid, name, gender))

with open(OUT_PATH, 'w', encoding='utf-8') as f:
    f.write("-- 自动生成，勿手动编辑\n")
    f.write("USE week_7_server;\n\n")
    for i in range(0, len(rows), BATCH):
        batch = rows[i:i + BATCH]
        vals  = ',\n    '.join(f"({r[0]}, '{r[1]}', '{r[2]}')" for r in batch)
        f.write(f"INSERT INTO Teacher (tid, name, gender) VALUES\n    {vals};\n\n")

print(f"共生成 {len(rows)} 条教师记录 -> {OUT_PATH}")
