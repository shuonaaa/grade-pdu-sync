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
    courseName   VARCHAR(100) NOT NULL,
    credit       INT,
    courseHour   INT,
    ScoreType    ENUM('percentile', 'fivePoint', 'examCheck'),
    PRIMARY KEY (courseNumber)
);

CREATE TABLE SC (
    sid          INT  NOT NULL,
    tid          INT  NOT NULL,
    courseNumber INT  NOT NULL,
    period       ENUM('last_per', 'next_per'),
    RealScore    INT,
    status       ENUM('pending','submitted','confirmed','finished','exception')  NOT NULL,
    PRIMARY KEY (sid, tid, courseNumber),
    FOREIGN KEY (sid)          REFERENCES Student(sid),
    FOREIGN KEY (tid)          REFERENCES Teacher(tid),
    FOREIGN KEY (courseNumber) REFERENCES Course(courseNumber)
);
