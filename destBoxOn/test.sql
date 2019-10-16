-- phpMyAdmin SQL Dump
-- version 4.8.4
-- https://www.phpmyadmin.net/
--
-- Host: localhost
-- Generation Time: Oct 16, 2019 at 05:21 AM
-- Server version: 5.7.27-0ubuntu0.18.04.1
-- PHP Version: 7.2.19-0ubuntu0.18.04.2

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET AUTOCOMMIT = 0;
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `test`
--

-- --------------------------------------------------------

--
-- Table structure for table `destbox_devices`
--

CREATE TABLE `destbox_devices` (
  `did` int(5) UNSIGNED NOT NULL,
  `password` tinytext NOT NULL,
  `software_version` tinytext NOT NULL,
  `eeprom_version` tinyint(3) UNSIGNED NOT NULL,
  `maintainer_mail` varchar(300) NOT NULL,
  `handbook_number` tinyint(4) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `destbox_keys`
--

CREATE TABLE `destbox_keys` (
  `kid` int(5) UNSIGNED NOT NULL,
  `did` int(5) UNSIGNED NOT NULL,
  `used` datetime DEFAULT NULL,
  `crypt` varchar(30) NOT NULL,
  `iv` varbinary(255) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;


--
-- Indexes for dumped tables
--

--
-- Indexes for table `destbox_devices`
--
ALTER TABLE `destbox_devices`
  ADD PRIMARY KEY (`did`);

--
-- Indexes for table `destbox_keys`
--
ALTER TABLE `destbox_keys`
  ADD PRIMARY KEY (`did`,`kid`) USING BTREE;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
