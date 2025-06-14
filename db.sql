CREATE TABLE users (
    id INT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(32) UNIQUE,
    password VARCHAR(64) NOT NULL,
    email VARCHAR(64),
    token VARCHAR(64),
    is_online TINYINT(1) DEFAULT 0,
    last_login DATETIME,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE rooms (
    id INT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(32) NOT NULL,
    password VARCHAR(64),
    status ENUM('waiting','playing','full') DEFAULT 'waiting',
    created_by INT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
);

CREATE TABLE room_members (
    id INT AUTO_INCREMENT PRIMARY KEY,
    room_id INT NOT NULL,
    user_id INT NOT NULL,
    role ENUM('black','white','spectator') DEFAULT 'spectator',
    ready TINYINT(1) DEFAULT 0,
    joined_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    INDEX(room_id),
    INDEX(user_id)
);

CREATE TABLE games (
    id INT AUTO_INCREMENT PRIMARY KEY,
    room_id INT NOT NULL,
    black_user INT NOT NULL,
    white_user INT NOT NULL,
    winner INT,
    reason VARCHAR(32),
    moves TEXT,
    started_at DATETIME,
    ended_at DATETIME
);

CREATE TABLE room_messages (
    id INT AUTO_INCREMENT PRIMARY KEY,
    room_id INT NOT NULL,
    user_id INT NOT NULL,
    content VARCHAR(512) NOT NULL,
    sent_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    INDEX(room_id),
    INDEX(user_id)
);
