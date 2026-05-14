CREATE TABLE Student (
    sid    INT          NOT NULL,
    name   VARCHAR(100) NOT NULL,
    birth  DATE,
    gender VARCHAR(10),
    PRIMARY KEY (sid)
);

CREATE TABLE Teacher (
    tid    INT          NOT NULL,
    name   VARCHAR(100) NOT NULL,
    gender VARCHAR(10),
    PRIMARY KEY (tid)
);

CREATE TABLE Course (
    courseNumber INT          NOT NULL,
    period       ENUM('last_per', 'next_per') NOT NULL,
    courseName   VARCHAR(100) NOT NULL,
    credit       INT,
    courseHour   INT,
    ScoreType    ENUM('percentile', 'fivePoint', 'examCheck'),
    PRIMARY KEY (courseNumber, period)
);

CREATE TABLE SC (
    sid          INT  NOT NULL,
    tid          INT  NOT NULL,
    courseNumber INT  NOT NULL,
    period       ENUM('last_per', 'next_per') NOT NULL,
    RealScore    INT,
    status       ENUM('pending','submitted','checked','confirmed','finished','exception','error')  NOT NULL DEFAULT 'pending',
    sent         TINYINT NOT NULL DEFAULT 0,
    PRIMARY KEY (sid, tid, courseNumber, period),
    FOREIGN KEY (sid)          REFERENCES Student(sid),
    FOREIGN KEY (tid)          REFERENCES Teacher(tid),
    FOREIGN KEY (courseNumber, period) REFERENCES Course(courseNumber, period)
);
