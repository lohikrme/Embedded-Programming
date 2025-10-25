import express from 'express';

const app = express();
const PORT = 3000;

// create a webhook to some discord server and copy its url here
const DISCORD_WEBHOOK_URL =
    'https://discord.com/api/webhooks/1425444507582857330/b7AdSY0kkdqBntkKh4Phe1z6etQVDPrioxVAkZR0AwQH-2zIr6EOxYxkxkZrBTt-Fd5h';

app.use(express.json());

app.post('/notify', (req, res) => {
    const message = req.body.message;

    if (!message) {
        return res.status(400).json({ error: 'Message is required' });
    }

    fetch(DISCORD_WEBHOOK_URL, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ content: message }),
    })
        .then((response) => {
            // if not receive ok response, something went wrong
            if (!response.ok) {
                throw new Error(`Discord responded with status ${response.status}`);
            }
            res.json({ status: 'Message sent...' });
        })
        .catch((error) => {
            console.error('Error sending request to Discord server\n', error);
            res.status(500).json({
                error: 'Failed sending your message to Discord server',
            });
        });
});

app.listen(PORT, () => {
    console.log('Server is listening on port', PORT);
});
