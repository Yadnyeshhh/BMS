-- SQL Setup Queries for C++ Banking System
-- Run this script in your MySQL client to initialize the database and tables.

-- Create the database if it doesn't already exist
CREATE DATABASE IF NOT EXISTS bank_db;

-- Switch to the bank_db database
USE bank_db;

-- Create the accounts table
CREATE TABLE IF NOT EXISTS accounts (
    account_id INT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    email VARCHAR(255) UNIQUE,
    password VARCHAR(255) NOT NULL,
    balance DOUBLE NOT NULL DEFAULT 0.0
);
