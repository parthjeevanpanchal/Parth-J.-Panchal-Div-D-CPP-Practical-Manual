const http = require('http');
const fs = require('fs');
const path = require('path');
const url = require('url');

const PORT = process.env.PORT || 3000;
const DATA_DIR = path.join(__dirname, 'data');
const INVENTORY_FILE = path.join(DATA_DIR, 'inventory.csv');
const TRANSACTIONS_FILE = path.join(DATA_DIR, 'transactions.csv');
const PUBLIC_DIR = path.join(__dirname, 'public');

// Ensure data directory exists
if (!fs.existsSync(DATA_DIR)) {
    fs.mkdirSync(DATA_DIR, { recursive: true });
}

// Helper: Parse CSV Line taking quotes into account
function parseCSVLine(line) {
    const fields = [];
    let field = '';
    let inQuotes = false;
    for (let i = 0; i < line.length; i++) {
        const ch = line[i];
        if (ch === '"') {
            inQuotes = !inQuotes;
        } else if (ch === ',' && !inQuotes) {
            fields.push(field.trim());
            field = '';
        } else if (ch !== '\r') {
            field += ch;
        }
    }
    fields.push(field.trim());
    return fields;
}

// Read inventory items from CSV
function readInventory() {
    if (!fs.existsSync(INVENTORY_FILE)) return [];
    const content = fs.readFileSync(INVENTORY_FILE, 'utf-8');
    const lines = content.split('\n').filter(l => l.trim().length > 0);
    const items = [];

    for (let i = 1; i < lines.length; i++) { // Skip header
        const tokens = parseCSVLine(lines[i]);
        if (tokens.length < 6) continue;
        const type = tokens[0];
        const id = parseInt(tokens[1], 10);
        const name = tokens[2].replace(/^"|"$/g, '');
        const quantity = parseInt(tokens[3], 10);
        const price = parseFloat(tokens[4]);
        const category = tokens[5].replace(/^"|"$/g, '');
        const extra1 = tokens[6] ? tokens[6].replace(/^"|"$/g, '') : 'N/A';
        const extra2 = tokens[7] ? tokens[7].replace(/^"|"$/g, '') : 'N/A';

        const item = {
            id,
            name,
            type,
            category,
            quantity,
            price,
            valuation: parseFloat((quantity * price).toFixed(2))
        };

        if (type === 'Perishable') {
            item.expiryDate = extra1 !== 'N/A' ? extra1 : '2026-12-31';
            item.storageTemp = extra2 !== 'N/A' ? parseFloat(extra2) : 4.0;
        } else if (type === 'Electronics') {
            item.warrantyMonths = extra1 !== 'N/A' ? parseInt(extra1, 10) : 12;
            item.brand = extra2 !== 'N/A' ? extra2 : 'Generic';
        }

        items.push(item);
    }
    return items;
}

// Write inventory items to CSV
function writeInventory(items) {
    let csv = 'Type,ID,Name,Quantity,Price,Category,Extra1,Extra2\n';
    for (const item of items) {
        let extra1 = 'N/A';
        let extra2 = 'N/A';
        if (item.type === 'Perishable') {
            extra1 = item.expiryDate || '2026-12-31';
            extra2 = item.storageTemp !== undefined ? item.storageTemp : '4.0';
        } else if (item.type === 'Electronics') {
            extra1 = item.warrantyMonths !== undefined ? item.warrantyMonths : '12';
            extra2 = item.brand || 'Generic';
        }

        csv += `${item.type},${item.id},"${item.name}",${item.quantity},${item.price.toFixed(2)},"${item.category}",${extra1},${extra2}\n`;
    }
    fs.writeFileSync(INVENTORY_FILE, csv, 'utf-8');
}

// Read transactions from CSV
function readTransactions() {
    if (!fs.existsSync(TRANSACTIONS_FILE)) return [];
    const content = fs.readFileSync(TRANSACTIONS_FILE, 'utf-8');
    const lines = content.split('\n').filter(l => l.trim().length > 0);
    const transactions = [];

    for (let i = 1; i < lines.length; i++) { // Skip header
        const tokens = parseCSVLine(lines[i]);
        if (tokens.length < 7) continue;
        transactions.push({
            txId: parseInt(tokens[0], 10),
            timestamp: tokens[1].replace(/^"|"$/g, ''),
            itemId: parseInt(tokens[2], 10),
            itemName: tokens[3].replace(/^"|"$/g, ''),
            type: tokens[4],
            quantityChange: parseInt(tokens[5], 10),
            resultingStock: parseInt(tokens[6], 10),
            notes: tokens[7] ? tokens[7].replace(/^"|"$/g, '') : ''
        });
    }
    return transactions;
}

// Append transaction to CSV
function logTransaction(itemId, itemName, type, qtyChange, resultingStock, notes) {
    const transactions = readTransactions();
    const nextTxId = transactions.length > 0
        ? Math.max(...transactions.map(t => t.txId || 0)) + 1
        : 1;

    const now = new Date();
    const timestamp = now.toISOString().replace('T', ' ').substring(0, 19);

    const txLine = `${nextTxId},"${timestamp}",${itemId},"${itemName}",${type},${qtyChange},${resultingStock},"${notes || ''}"\n`;

    if (!fs.existsSync(TRANSACTIONS_FILE)) {
        fs.writeFileSync(TRANSACTIONS_FILE, 'TxID,Timestamp,ItemID,ItemName,Type,QuantityChange,ResultingStock,Notes\n', 'utf-8');
    }
    fs.appendFileSync(TRANSACTIONS_FILE, txLine, 'utf-8');
}

// Request Helper: parse JSON body
function getRequestBody(req) {
    return new Promise((resolve, reject) => {
        let body = '';
        req.on('data', chunk => { body += chunk; });
        req.on('end', () => {
            try {
                resolve(body ? JSON.parse(body) : {});
            } catch (err) {
                reject(err);
            }
        });
    });
}

// MIME types for static files
const MIME_TYPES = {
    '.html': 'text/html',
    '.css': 'text/css',
    '.js': 'application/javascript',
    '.json': 'application/json',
    '.png': 'image/png',
    '.jpg': 'image/jpeg',
    '.svg': 'image/svg+xml',
    '.ico': 'image/x-icon'
};

const server = http.createServer(async (req, res) => {
    const parsedUrl = url.parse(req.url, true);
    const pathname = parsedUrl.pathname;
    const method = req.method;

    // Set CORS headers
    res.setHeader('Access-Control-Allow-Origin', '*');
    res.setHeader('Access-Control-Allow-Methods', 'GET, POST, PUT, DELETE, OPTIONS');
    res.setHeader('Access-Control-Allow-Headers', 'Content-Type');

    if (method === 'OPTIONS') {
        res.writeHead(204);
        res.end();
        return;
    }

    // Helper: JSON response
    const sendJSON = (data, statusCode = 200) => {
        res.writeHead(statusCode, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify(data));
    };

    try {
        // --- API ROUTES ---

        // GET /api/items
        if (pathname === '/api/items' && method === 'GET') {
            const items = readInventory();
            sendJSON(items);
            return;
        }

        // POST /api/items (Add Item)
        if (pathname === '/api/items' && method === 'POST') {
            const body = await getRequestBody(req);
            const items = readInventory();

            const id = parseInt(body.id, 10);
            const name = (body.name || '').trim();
            const quantity = parseInt(body.quantity, 10);
            const price = parseFloat(body.price);
            const category = (body.category || 'General').trim();
            const type = body.type || 'Standard';

            if (!id || id <= 0) return sendJSON({ error: 'Item ID must be a positive integer.' }, 400);
            if (!name) return sendJSON({ error: 'Item name cannot be empty.' }, 400);
            if (isNaN(quantity) || quantity < 0) return sendJSON({ error: 'Quantity cannot be negative.' }, 400);
            if (isNaN(price) || price < 0) return sendJSON({ error: 'Price cannot be negative.' }, 400);

            if (items.some(it => it.id === id)) {
                return sendJSON({ error: `DuplicateItemException: Item ID ${id} already exists!` }, 400);
            }

            const newItem = {
                id,
                name,
                type,
                category,
                quantity,
                price,
                valuation: parseFloat((quantity * price).toFixed(2))
            };

            if (type === 'Perishable') {
                newItem.expiryDate = body.expiryDate || '2026-12-31';
                newItem.storageTemp = parseFloat(body.storageTemp) || 4.0;
            } else if (type === 'Electronics') {
                newItem.warrantyMonths = parseInt(body.warrantyMonths, 10) || 12;
                newItem.brand = body.brand || 'Generic';
            }

            items.push(newItem);
            writeInventory(items);
            logTransaction(id, name, 'INITIAL', quantity, quantity, 'Created via Web Portal');

            sendJSON({ success: true, item: newItem }, 201);
            return;
        }

        // POST /api/items/:id/stock-in
        if (pathname.match(/^\/api\/items\/\d+\/stock-in$/) && method === 'POST') {
            const id = parseInt(pathname.split('/')[3], 10);
            const body = await getRequestBody(req);
            const amount = parseInt(body.amount, 10);
            const notes = body.notes || 'Replenishment';

            if (isNaN(amount) || amount <= 0) {
                return sendJSON({ error: 'Stock-in quantity must be greater than zero.' }, 400);
            }

            const items = readInventory();
            const item = items.find(it => it.id === id);
            if (!item) return sendJSON({ error: `ItemNotFoundException: Item ID ${id} not found.` }, 404);

            item.quantity += amount;
            item.valuation = parseFloat((item.quantity * item.price).toFixed(2));

            writeInventory(items);
            logTransaction(item.id, item.name, 'STOCK_IN', amount, item.quantity, notes);

            sendJSON({ success: true, item, message: `Successfully added ${amount} units.` });
            return;
        }

        // POST /api/items/:id/stock-out (Guaranteed Negative Stock Prevention)
        if (pathname.match(/^\/api\/items\/\d+\/stock-out$/) && method === 'POST') {
            const id = parseInt(pathname.split('/')[3], 10);
            const body = await getRequestBody(req);
            const amount = parseInt(body.amount, 10);
            const notes = body.notes || 'Dispatch';

            if (isNaN(amount) || amount <= 0) {
                return sendJSON({ error: 'Stock-out quantity must be greater than zero.' }, 400);
            }

            const items = readInventory();
            const item = items.find(it => it.id === id);
            if (!item) return sendJSON({ error: `ItemNotFoundException: Item ID ${id} not found.` }, 404);

            // CRITICAL CHECK: Prevent negative stock
            if (amount > item.quantity) {
                return sendJSON({
                    error: `InsufficientStockException: Requested dispatch (${amount}) exceeds available stock (${item.quantity}) for Item ID ${id}. Operation aborted to prevent negative stock!`
                }, 400);
            }

            item.quantity -= amount;
            item.valuation = parseFloat((item.quantity * item.price).toFixed(2));

            writeInventory(items);
            logTransaction(item.id, item.name, 'STOCK_OUT', -amount, item.quantity, notes);

            sendJSON({ success: true, item, message: `Successfully dispatched ${amount} units.` });
            return;
        }

        // PUT /api/items/:id (Update Item)
        if (pathname.match(/^\/api\/items\/\d+$/) && method === 'PUT') {
            const id = parseInt(pathname.split('/')[3], 10);
            const body = await getRequestBody(req);
            const items = readInventory();
            const item = items.find(it => it.id === id);
            if (!item) return sendJSON({ error: `ItemNotFoundException: Item ID ${id} not found.` }, 404);

            if (body.name) item.name = body.name.trim();
            if (body.category) item.category = body.category.trim();
            if (body.price !== undefined) {
                const newPrice = parseFloat(body.price);
                if (newPrice < 0) return sendJSON({ error: 'Price cannot be negative.' }, 400);
                item.price = newPrice;
            }
            if (item.type === 'Perishable') {
                if (body.expiryDate) item.expiryDate = body.expiryDate;
                if (body.storageTemp !== undefined) item.storageTemp = parseFloat(body.storageTemp);
            } else if (item.type === 'Electronics') {
                if (body.warrantyMonths !== undefined) item.warrantyMonths = parseInt(body.warrantyMonths, 10);
                if (body.brand) item.brand = body.brand;
            }

            item.valuation = parseFloat((item.quantity * item.price).toFixed(2));
            writeInventory(items);
            logTransaction(item.id, item.name, 'PRICE_UPDATE', 0, item.quantity, 'Updated attributes via Web Portal');

            sendJSON({ success: true, item });
            return;
        }

        // DELETE /api/items/:id
        if (pathname.match(/^\/api\/items\/\d+$/) && method === 'DELETE') {
            const id = parseInt(pathname.split('/')[3], 10);
            let items = readInventory();
            const item = items.find(it => it.id === id);
            if (!item) return sendJSON({ error: `ItemNotFoundException: Item ID ${id} not found.` }, 404);

            items = items.filter(it => it.id !== id);
            writeInventory(items);
            sendJSON({ success: true, message: `Item ID ${id} removed.` });
            return;
        }

        // GET /api/transactions
        if (pathname === '/api/transactions' && method === 'GET') {
            const transactions = readTransactions();
            sendJSON(transactions.reverse()); // most recent first
            return;
        }

        // GET /api/stats
        if (pathname === '/api/stats' && method === 'GET') {
            const items = readInventory();
            const totalItems = items.length;
            const totalUnits = items.reduce((acc, it) => acc + it.quantity, 0);
            const totalValuation = items.reduce((acc, it) => acc + it.valuation, 0);
            const lowStockCount = items.filter(it => it.quantity <= 5).length;
            const perishableCount = items.filter(it => it.type === 'Perishable').length;
            const electronicsCount = items.filter(it => it.type === 'Electronics').length;

            sendJSON({
                totalItems,
                totalUnits,
                totalValuation: parseFloat(totalValuation.toFixed(2)),
                lowStockCount,
                perishableCount,
                electronicsCount
            });
            return;
        }

        // --- STATIC FILE SERVING ---
        let filePath = path.join(PUBLIC_DIR, pathname === '/' ? 'index.html' : pathname);

        // Security check
        if (!filePath.startsWith(PUBLIC_DIR)) {
            res.writeHead(403);
            res.end('Access Denied');
            return;
        }

        if (fs.existsSync(filePath) && fs.statSync(filePath).isFile()) {
            const ext = path.extname(filePath).toLowerCase();
            const contentType = MIME_TYPES[ext] || 'application/octet-stream';
            res.writeHead(200, { 'Content-Type': contentType });
            fs.createReadStream(filePath).pipe(res);
            return;
        }

        // Fallback to index.html for SPA
        const indexPath = path.join(PUBLIC_DIR, 'index.html');
        if (fs.existsSync(indexPath)) {
            res.writeHead(200, { 'Content-Type': 'text/html' });
            fs.createReadStream(indexPath).pipe(res);
            return;
        }

        res.writeHead(404, { 'Content-Type': 'text/plain' });
        res.end('404 Not Found');

    } catch (err) {
        console.error('Server error:', err);
        sendJSON({ error: 'Internal Server Error: ' + err.message }, 500);
    }
});

server.listen(PORT, () => {
    console.log(`================================================================`);
    console.log(`  Inventory Manager Web Application is live at:`);
    console.log(`  http://localhost:${PORT}`);
    console.log(`================================================================`);
});
