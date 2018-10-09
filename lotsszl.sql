/*
Navicat MySQL Data Transfer

Source Server         : 腾讯 Mysql 独立
Source Server Version : 50718
Source Host           : 
Source Database       : lotsszl

Target Server Type    : MYSQL
Target Server Version : 50718
File Encoding         : 65001

Date: 2018-10-07 23:47:45
*/

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for data_table
-- ----------------------------
DROP TABLE IF EXISTS `data_table`;
CREATE TABLE `data_table` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `sensorID` varchar(255) NOT NULL,
  `date_timer` datetime NOT NULL ON UPDATE CURRENT_TIMESTAMP,
  `illumination` double NOT NULL,
  `humidity` double NOT NULL,
  ` temperature` double NOT NULL,
  PRIMARY KEY (`id`),
  KEY `sensorID_waijian` (`sensorID`),
  CONSTRAINT `sensorID_waijian` FOREIGN KEY (`sensorID`) REFERENCES `user_sensor` (`sensorID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for user_sensor
-- ----------------------------
DROP TABLE IF EXISTS `user_sensor`;
CREATE TABLE `user_sensor` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `user` varchar(255) CHARACTER SET utf8mb4 NOT NULL,
  `sensorID` varchar(255) NOT NULL,
  `key` varchar(255) NOT NULL,
  PRIMARY KEY (`id`,`user`,`sensorID`),
  KEY `user_suoying` (`user`),
  KEY `sensorID_suoying` (`sensorID`) USING BTREE,
  CONSTRAINT `user_waijian` FOREIGN KEY (`user`) REFERENCES `user_table` (`user`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for user_table
-- ----------------------------
DROP TABLE IF EXISTS `user_table`;
CREATE TABLE `user_table` (
  `id` int(10) NOT NULL AUTO_INCREMENT,
  `user` varchar(255) NOT NULL COMMENT '用户',
  `Email` varchar(255) DEFAULT NULL,
  `nikename` varchar(100) NOT NULL COMMENT '昵称',
  `pwd` varchar(255) NOT NULL COMMENT '密码',
  `role` int(10) NOT NULL DEFAULT '1' COMMENT '角色  admin=0  user=1',
  PRIMARY KEY (`id`,`user`),
  KEY `user_shuoying` (`user`) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=18 DEFAULT CHARSET=utf8mb4;
