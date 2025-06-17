-- MySQL dump 10.13  Distrib 5.7.31, for Linux (x86_64)
--
-- 主机: localhost    数据库: chat
-- ------------------------------------------------------
-- 服务器版本	8

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- 群组表结构
--

DROP TABLE IF EXISTS `allgroup`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `allgroup` (
  `id` int(11) NOT NULL AUTO_INCREMENT,  -- 群组ID，主键，自增
  `groupname` varchar(50) CHARACTER SET latin1 NOT NULL,  -- 群组名称，不能为空
  `groupdesc` varchar(200) CHARACTER SET latin1 DEFAULT '',  -- 群组描述，默认为空
  PRIMARY KEY (`id`),
  UNIQUE KEY `groupname` (`groupname`)  -- 群组名称唯一索引
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- 群组表数据导入
--

LOCK TABLES `allgroup` WRITE;
/*!40000 ALTER TABLE `allgroup` DISABLE KEYS */;
INSERT INTO `allgroup` VALUES (1,'C++ chat project','start develop a chat project');
/*!40000 ALTER TABLE `allgroup` ENABLE KEYS */;
UNLOCK TABLES;

--
-- 好友关系表结构
--

DROP TABLE IF EXISTS `friend`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `friend` (
  `userid` int(11) NOT NULL,     -- 用户ID
  `friendid` int(11) NOT NULL,   -- 好友ID
  KEY `userid` (`userid`,`friendid`)  -- 联合索引
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- 好友关系表数据导入
--

LOCK TABLES `friend` WRITE;
/*!40000 ALTER TABLE `friend` DISABLE KEYS */;
INSERT INTO `friend` VALUES (13,15),(13,21),(21,13),(21,15);
/*!40000 ALTER TABLE `friend` ENABLE KEYS */;
UNLOCK TABLES;

--
-- 群组用户关系表结构
--

DROP TABLE IF EXISTS `groupuser`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `groupuser` (
  `groupid` int(11) NOT NULL,    -- 群组ID
  `userid` int(11) NOT NULL,     -- 用户ID
  `grouprole` enum('creator','normal') CHARACTER SET latin1 DEFAULT NULL,  -- 群组角色：创建者或普通成员
  KEY `groupid` (`groupid`,`userid`)  -- 联合索引
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- 群组用户关系表数据导入
--

LOCK TABLES `groupuser` WRITE;
/*!40000 ALTER TABLE `groupuser` DISABLE KEYS */;
INSERT INTO `groupuser` VALUES (1,13,'creator'),(1,21,'normal'),(1,19,'normal');
/*!40000 ALTER TABLE `groupuser` ENABLE KEYS */;
UNLOCK TABLES;

--
-- 离线消息表结构
--

DROP TABLE IF EXISTS `offlinemessage`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `offlinemessage` (
  `userid` int(11) NOT NULL,     -- 接收消息的用户ID
  `message` varchar(500) NOT NULL  -- 离线消息内容（JSON格式）
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- 离线消息表数据导入
--

LOCK TABLES `offlinemessage` WRITE;
/*!40000 ALTER TABLE `offlinemessage` DISABLE KEYS */;
INSERT INTO `offlinemessage` VALUES (19,'{\"groupid\":1,\"id\":21,\"msg\":\"hello\",\"msgid\":10,\"name\":\"gao yang\",\"time\":\"2020-02-22 00:43:59\"}'),(19,'{\"groupid\":1,\"id\":21,\"msg\":\"helo!!!\",\"msgid\":10,\"name\":\"gao yang\",\"time\":\"2020-02-22 22:43:21\"}'),(19,'{\"groupid\":1,\"id\":13,\"msg\":\"hahahahaha\",\"msgid\":10,\"name\":\"zhang san\",\"time\":\"2020-02-22 22:59:56\"}'),(19,'{\"groupid\":1,\"id\":13,\"msg\":\"hahahahaha\",\"msgid\":10,\"name\":\"zhang san\",\"time\":\"2020-02-23 17:59:26\"}'),(19,'{\"groupid\":1,\"id\":21,\"msg\":\"wowowowowow\",\"msgid\":10,\"name\":\"gao yang\",\"time\":\"2020-02-23 17:59:34\"}');
/*!40000 ALTER TABLE `offlinemessage` ENABLE KEYS */;
UNLOCK TABLES;

--
-- 用户表结构
--

DROP TABLE IF EXISTS `user`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `user` (
  `id` int(11) NOT NULL AUTO_INCREMENT,  -- 用户ID，主键，自增
  `name` varchar(50) DEFAULT NULL,       -- 用户名
  `password` varchar(50) DEFAULT NULL,   -- 用户密码
  `state` enum('online','offline') CHARACTER SET latin1 DEFAULT 'offline',  -- 用户状态：在线或离线
  PRIMARY KEY (`id`),
  UNIQUE KEY `name` (`name`)  -- 用户名唯一索引
) ENGINE=InnoDB AUTO_INCREMENT=22 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- 用户表数据导入
--

LOCK TABLES `user` WRITE;
/*!40000 ALTER TABLE `user` DISABLE KEYS */;
INSERT INTO `user` VALUES (13,'zhang san','123456','online'),(15,'li si','666666','offline'),(16,'liu shuo','123456','offline'),(18,'wu yang','123456','offline'),(19,'pi pi','123456','offline'),(21,'gao yang','123456','offline');
/*!40000 ALTER TABLE `user` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- 数据导出完成于 2021-07-28 14:36:11
