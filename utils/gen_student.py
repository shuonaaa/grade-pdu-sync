import random
import datetime
import os
import itertools as _it


GRADES              = [22, 23, 24, 25]   # 在校年级
DEPT_COUNT          = 10                  # 系号 01-10
MAJOR_PER_DEPT      = 3                   # 每系专业数 01-03
CLASS_PER_MAJOR     = 4                   # 每专业班数 1-4
STUDENTS_PER_CLASS  = 40                  # 每班人数 01-40
# 总计: 4 × 10 × 3 × 4 × 40 = 19,200 人


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



_M1 = '浩宇博志子子轩宇泽浩然博文嘉睿思辰逸俊伟杰鹏涛飞龙明阳彬彦哲翰铭瑞昕昊天梓恒源祺宸烁磊峰波海华永志宏若廷景易康健辉耀鑫森淼焱旭东方涵桐楠楷煜炜烨霖熙峻宁瑜瑾墨诚恩善绍承继秉延庆吉祥泰和正义仁德信智勇礼孝贤良尊盛昌隆达顺利恒安邦宁远平经纬乾坤世超光新朝宝旭昌春亮军松林建刚强胜雄武彪壮斌彪冠君伦耀修鑫允凡顺全厚才清杰人伯承夫群丹启令保永树义盛士富大友云军金有生社风铁占红秉长雪守金延秉政汝治焕起作相存宗培成百河瑞克邦振宪学太俊守善春耀文承昊言希先良泽安林朝永田保承秀礼慧孝和庆守德宝树道凯贵昌俊文泽俊天明昊梓子轩宇泽浩然博文睿辰逸俊杰豪伟强涛鹏飞龙阳明林彬彦哲翰铭瑞昕嘉思雨天一梓瀚皓毅恒源祺宸晨烁磊峰波海华永志宏若廷景易康健辉耀鑫森淼炎旭东方涵桐楠楷煜炜烨霖熙峻宁瑜瑾墨诚恩善绍承继秉延庆吉祥泰和正义仁德信鸿锦旭思铭远建鹏翔睿晨云龙威凯杰磊强飞勇辉涛刚峰振成轩辰豪逸诚恒毅智达弘瀚彦景嘉哲烨霖晗宸凌峻骏熙瑞耀炜焱炳炫正广光煜祺锐晋健承启兆嘉俊安乐'
_M2 = '然轩文远豪辰宇航哲明磊程杰辉飞嘉曦逸智达弘瀚彦景恒毅睿晨云翔涛峰强刚勇凯泽霖晗宸铭烨熙瑞耀炜光广正子轩宇泽浩然博文嘉睿思辰逸俊伟杰鹏涛飞龙明阳彬彦哲翰铭瑞昕昊天梓恒源祺宸烁磊峰波海华永志宏若廷景易康健辉耀鑫森淼焱旭东方涵桐楠楷煜炜烨霖熙峻宁瑜瑾墨诚恩善绍承继秉延庆吉祥泰和正义仁德信智勇礼孝贤良尊盛昌隆达顺利恒安邦宁远平经纬乾坤世超光新朝宝旭昌春亮军松林建刚强胜雄武彪壮斌彪冠君伦耀修鑫允凡顺全厚才清杰人伯承夫群丹启令保永树义盛士富大友云军金有生社风铁占红秉长雪守金延秉政汝治焕起作相存宗培成百河瑞克邦振宪学太俊守善春耀文承昊言希先良泽安林朝永田保承秀礼慧孝和庆守德宝树道凯贵昌俊鑫龙威峻骏炳炫焱煜祺锐晋健承启兆安乐成振博浩志子轩宇泽浩然博文睿辰逸俊杰豪伟强涛鹏飞龙阳明林彬彦哲翰铭瑞昕嘉思雨天一梓瀚皓毅恒源祺宸晨烁磊峰波海华永志宏若廷景易康健辉耀鑫森淼炎旭东方涵桐楠楷煜炜烨霖熙峻宁瑜瑾墨诚恩善绍承继秉延庆吉祥泰和正义仁德信思建梓天昊俊文明'

_F1 = '梓欣雨诗若梦雪晓静思心婉语佳可慧安紫悦嘉婷雨欣怡佳琪诗涵梦瑶子萱紫妍嘉妮悦然静雯雪琳思琪雅婷晓彤可馨莉莹慧敏芳菲筱婷婉清若云思雨美琳艺涵梓涵语嫣玉洁丹妮心怡欣妍晓雯若溪翠芷文雅映依冰凌采嫣涵怡桐彤萱妍瑾清洁晴瑶薇琳菲璐颖冉宁蕊雯莹婕丽颜珍玉淑芳美艳华燕韵悠温柔婷凤秀英兰蓉莲霜露曼卿云月灵'
_F2 = '涵怡桐彤萱妍瑾清洁晴瑶薇琳菲璐颖冉宁蕊雯莹婕丽颜珍玉淑芳美艳华燕韵悠温柔婷雪晓静思心婉语佳可慧安紫悦嘉芷文雅映依冰采婷雨欣怡佳琪诗涵梦瑶子萱紫妍嘉妮悦然静雯雪琳思琪雅婷晓彤可馨莉莹慧敏芳菲筱婷婉清若云思雨美琳艺涵梓涵语嫣玉洁丹妮心怡欣妍晓雯若溪翠嫣若梦诗雨梓云凤秀英兰蓉莲霜露曼卿月灵琪欣'

def _gen_double(c1, c2, n):
    pool = list({a + b for a, b in _it.product(c1, c2) if a != b})
    random.shuffle(pool)
    return pool[:n]

def _gen_triple(singles, doubles, n):
    pool = list({s + d for s, d in _it.product(singles, doubles)})
    random.shuffle(pool)
    return pool[:n]

# 单字
_M_SINGLE = list('浩宇博志杰明辉涛磊鹏龙威凯翔睿晨峰强勇刚昊梓铭烨熙瑞耀炜祺宸')
_F_SINGLE = list('梅芳华英玲萍燕丽娜婷雪晴欣悦静美洁慧颖莹瑶萱彤怡涵琪菲璐')

# 双字
_MALE_DOUBLE   = _gen_double(_M1, _M2, 2000)
_FEMALE_DOUBLE = _gen_double(_F1, _F2, 2000)

# 三字
_MALE_TRIPLE   = _gen_triple(_M_SINGLE, _MALE_DOUBLE,   500)
_FEMALE_TRIPLE = _gen_triple(_F_SINGLE, _FEMALE_DOUBLE, 500)

# 比例：单字 15% / 双字 80% / 三字 5%
_NAME_WEIGHTS = [(1, 0.15), (2, 0.80), (3, 0.05)]


BIRTH_DIST = {
    22: [(2004, 80), (2003, 4), (2005, 4), (2002, 1), (2001, 1)],
    23: [(2005, 80), (2004, 4), (2006, 3), (2003, 1), (2002, 1), (2007, 1)],
    24: [(2006, 80), (2005, 4), (2007, 3), (2004, 1), (2003, 1), (2008, 1)],
    25: [(2007, 80), (2006, 4), (2008, 3), (2005, 71), (2004, 1), (2009, 1)],
}

def weighted_choice(dist):
    total = sum(w for _, w in dist)
    r = random.randint(1, total)
    cumul = 0
    for val, w in dist:
        cumul += w
        if r <= cumul:
            return val

def random_date(year):
    start = datetime.date(year, 1, 1)
    return start + datetime.timedelta(days=random.randint(0, 364))

def make_sid(grade, dept, major, cls, seq):
    return f"{grade:02d}{dept:02d}{major:02d}{cls:1d}{seq:02d}"

def make_name(gender):
    surname = random.choice(SURNAMES)
    r = random.random()
    if r < 0.15:
        first = random.choice(_M_SINGLE if gender == '男' else _F_SINGLE)
    elif r < 0.95:
        first = random.choice(_MALE_DOUBLE if gender == '男' else _FEMALE_DOUBLE)
    else:
        first = random.choice(_MALE_TRIPLE if gender == '男' else _FEMALE_TRIPLE)
    return surname + first

# ── 生成 ─────────────────────────────────────────────────
OUT_PATH = os.path.join(os.path.dirname(__file__), '../database/Student_insert.sql')
OUT_PATH = os.path.normpath(OUT_PATH)

BATCH = 500   # 每条 INSERT 包含的行数，避免单条 SQL 过长

rows = []
for grade in GRADES:
    for dept in range(1, DEPT_COUNT + 1):
        for major in range(1, MAJOR_PER_DEPT + 1):
            for cls in range(1, CLASS_PER_MAJOR + 1):
                for seq in range(1, STUDENTS_PER_CLASS + 1):
                    sid    = make_sid(grade, dept, major, cls, seq)
                    gender = random.choice(['男', '女'])
                    year   = weighted_choice(BIRTH_DIST[grade])
                    birth  = random_date(year)
                    name   = make_name(gender)
                    rows.append((sid, name, str(birth), gender))

with open(OUT_PATH, 'w', encoding='utf-8') as f:
    f.write("-- 自动生成，勿手动编辑\n")
    f.write("USE week_7_server;\n\n")

    for i in range(0, len(rows), BATCH):
        batch = rows[i:i + BATCH]
        vals  = ',\n    '.join(
            f"({r[0]}, '{r[1]}', '{r[2]}', '{r[3]}')" for r in batch
        )
        f.write(f"INSERT INTO Student (sid, name, birth, gender) VALUES\n    {vals};\n\n")

print(f"共生成 {len(rows)} 条学生记录 -> {OUT_PATH}")
