let username = "Sondre Rasmussen";

for (let i = 0; i < username.length; i++) {
    if (username[i] == ' ') {
        username[i] = '_';
    }
}

username = username.toLowerCase();

// sondre rasmussen