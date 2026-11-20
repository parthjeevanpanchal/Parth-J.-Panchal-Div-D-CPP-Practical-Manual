// StockMatrix India - Client Application (OOP & STL Edition)

let inventory = [];
let transactions = [];
let activeCategoryFilter = 'ALL';
let categoryValueChartInstance = null;
let categoryCountChartInstance = null;

// Currency Formatter for Indian Rupees (₹)
const inrFormatter = new Intl.NumberFormat('en-IN', {
    style: 'currency',
    currency: 'INR',
    minimumFractionDigits: 2,
    maximumFractionDigits: 2
});

function formatINR(amount) {
    if (isNaN(amount) || amount === null) return '₹0.00';
    return inrFormatter.format(amount);
}

// DOM References
const catalogTableBody = document.getElementById('catalogTableBody');
const txTableBody = document.getElementById('txTableBody');
const lowStockCardsGrid = document.getElementById('lowStockCardsGrid');
const catalogSearchInput = document.getElementById('catalogSearchInput');
const catalogTypeSelect = document.getElementById('catalogTypeSelect');
const catalogSortSelect = document.getElementById('catalogSortSelect');

// KPI Counters
const kpiTotalItems = document.getElementById('kpiTotalItems');
const kpiTotalUnits = document.getElementById('kpiTotalUnits');
const kpiTotalValuation = document.getElementById('kpiTotalValuation');
const kpiLowStock = document.getElementById('kpiLowStock');
const navLowStockCount = document.getElementById('navLowStockCount');
const navItemCount = document.getElementById('navItemCount');
const chipTotalValuation = document.getElementById('chipTotalValuation');

// Modals
const modalAdd = document.getElementById('modalAdd');
const modalStockIn = document.getElementById('modalStockIn');
const modalStockOut = document.getElementById('modalStockOut');

// Initialize on DOM Ready
document.addEventListener('DOMContentLoaded', () => {
    initTheme();
    initNavigation();
    initShortcuts();
    initModals();
    initCategoryChips();
    initSearchAndSort();
    fetchData();
    lucide.createIcons();
});

// Toast notification helper
function showToast(message, type = 'info') {
    const hub = document.getElementById('toastHub');
    const toast = document.createElement('div');
    toast.className = `toast-msg toast-${type}`;

    let icon = 'info';
    if (type === 'success') icon = 'check-circle-2';
    if (type === 'danger') icon = 'shield-alert';

    toast.innerHTML = `<i data-lucide="${icon}"></i> <span>${message}</span>`;
    hub.appendChild(toast);
    lucide.createIcons();

    setTimeout(() => {
        toast.style.opacity = '0';
        toast.style.transform = 'translateX(100%)';
        toast.style.transition = 'all 0.3s cubic-bezier(0.16, 1, 0.3, 1)';
        setTimeout(() => toast.remove(), 300);
    }, 4500);
}

// Theme Toggle (Dark / Light)
function initTheme() {
    const themeBtn = document.getElementById('themeToggleBtn');
    const savedTheme = localStorage.getItem('stockmatrix_theme') || 'light';
    document.documentElement.setAttribute('data-theme', savedTheme);
    updateThemeIcon(savedTheme);

    themeBtn.addEventListener('click', () => {
        const current = document.documentElement.getAttribute('data-theme');
        const next = current === 'dark' ? 'light' : 'dark';
        document.documentElement.setAttribute('data-theme', next);
        localStorage.setItem('stockmatrix_theme', next);
        updateThemeIcon(next);
        renderCharts();
    });
}

function updateThemeIcon(theme) {
    const icon = document.getElementById('themeIcon');
    if (theme === 'dark') {
        icon.setAttribute('data-lucide', 'sun');
    } else {
        icon.setAttribute('data-lucide', 'moon');
    }
    lucide.createIcons();
}

// Fetch Data from Server
async function fetchData() {
    try {
        const [itemsRes, txRes, statsRes] = await Promise.all([
            fetch('/api/items'),
            fetch('/api/transactions'),
            fetch('/api/stats')
        ]);

        if (!itemsRes.ok) throw new Error('Backend server returned an error');

        inventory = await itemsRes.json();
        transactions = await txRes.json();
        const stats = await statsRes.json();

        updateDashboardMetrics(stats);
        renderCatalog();
        renderLowStockView();
        renderTransactions();
        renderCharts();

        document.getElementById('backendStatusLabel').innerText = 'Connected (data/inventory.csv)';
        document.querySelector('.pulse-dot').style.background = '#10b981';
    } catch (err) {
        console.warn('Backend offline or error:', err);
        document.getElementById('backendStatusLabel').innerText = 'Offline Mode';
        document.querySelector('.pulse-dot').style.background = '#f59e0b';
        showToast('Running in local offline preview mode.', 'info');
    }
}

// Update Top Metric Counters
function updateDashboardMetrics(stats) {
    kpiTotalItems.innerText = stats.totalItems;
    navItemCount.innerText = stats.totalItems;
    kpiTotalUnits.innerText = stats.totalUnits.toLocaleString('en-IN');
    
    const formattedVal = formatINR(stats.totalValuation);
    kpiTotalValuation.innerText = formattedVal;
    chipTotalValuation.innerText = formattedVal;

    kpiLowStock.innerText = stats.lowStockCount;
    navLowStockCount.innerText = stats.lowStockCount;

    if (stats.lowStockCount > 0) {
        document.getElementById('kpiLowStockCard').style.borderColor = '#f59e0b';
    }
}

// Render Main Inventory Table with Filtering & Sorting
function renderCatalog() {
    const query = (catalogSearchInput.value || '').trim().toLowerCase();
    const typeSelect = catalogTypeSelect.value;
    const sortSelect = catalogSortSelect.value;

    let filtered = inventory.filter(item => {
        // OOP Type filter
        const matchesType = (typeSelect === 'ALL' || item.type === typeSelect);
        
        // Category chip filter
        let matchesChip = true;
        if (activeCategoryFilter === 'LOW_STOCK') {
            matchesChip = (item.quantity <= 5);
        } else if (activeCategoryFilter !== 'ALL') {
            matchesChip = (item.category.toLowerCase() === activeCategoryFilter.toLowerCase());
        }

        // Query filter
        const matchesQuery = !query ||
            item.name.toLowerCase().includes(query) ||
            item.id.toString().includes(query) ||
            item.category.toLowerCase().includes(query) ||
            (item.brand && item.brand.toLowerCase().includes(query));

        return matchesType && matchesChip && matchesQuery;
    });

    // Sort items
    filtered.sort((a, b) => {
        switch (sortSelect) {
            case 'id_asc': return a.id - b.id;
            case 'id_desc': return b.id - a.id;
            case 'name_asc': return a.name.localeCompare(b.name);
            case 'price_desc': return b.price - a.price;
            case 'price_asc': return a.price - b.price;
            case 'qty_desc': return b.quantity - a.quantity;
            case 'qty_asc': return a.quantity - b.quantity;
            default: return a.id - b.id;
        }
    });

    catalogTableBody.innerHTML = '';

    if (filtered.length === 0) {
        catalogTableBody.innerHTML = `
            <tr>
                <td colspan="9" style="text-align: center; padding: 48px; color: var(--text-muted);">
                    <i data-lucide="package-x" style="width: 42px; height: 42px; display: block; margin: 0 auto 12px; opacity: 0.4;"></i>
                    <strong style="font-size: 1.05rem;">No matching products found</strong>
                    <p style="font-size: 0.85rem; margin-top: 4px;">Try adjusting your search criteria or category filter.</p>
                </td>
            </tr>
        `;
        document.getElementById('tableSummaryText').innerText = `Showing 0 of ${inventory.length} products`;
        lucide.createIcons();
        return;
    }

    filtered.forEach(item => {
        const tr = document.createElement('tr');

        // OOP Type Badge
        let typeBadgeClass = 'badge-standard';
        let typeIcon = 'box';
        if (item.type === 'Perishable') {
            typeBadgeClass = 'badge-perishable';
            typeIcon = 'clock-3';
        } else if (item.type === 'Electronics') {
            typeBadgeClass = 'badge-electronics';
            typeIcon = 'cpu';
        }

        // Stock Progress & Health
        let fillClass = 'fill-emerald';
        let stockLabel = 'In Stock';
        let pct = Math.min(100, Math.round((item.quantity / 50) * 100));

        if (item.quantity === 0) {
            fillClass = 'fill-red';
            stockLabel = 'Out of Stock';
            pct = 0;
        } else if (item.quantity <= 5) {
            fillClass = 'fill-amber';
            stockLabel = 'Low Stock';
            pct = Math.max(12, Math.round((item.quantity / 50) * 100));
        }

        // Specific Attributes (Polymorphism)
        let specialDetails = '<span style="color:var(--text-subtle);">&mdash;</span>';
        if (item.type === 'Perishable') {
            specialDetails = `<span style="font-size:0.8rem; display:inline-flex; align-items:center; gap:4px;" title="Shelf Life & Temp">
                <i data-lucide="snowflake" style="width:13px; color:var(--emerald-600);"></i> Exp: ${item.expiryDate} (${item.storageTemp}&deg;C)
            </span>`;
        } else if (item.type === 'Electronics') {
            specialDetails = `<span style="font-size:0.8rem; display:inline-flex; align-items:center; gap:4px;" title="Warranty Period & Brand">
                <i data-lucide="shield-check" style="width:13px; color:var(--violet-600);"></i> ${item.warrantyMonths}m war. &bull; ${item.brand}
            </span>`;
        }

        tr.innerHTML = `
            <td class="item-id-cell">#${item.id}</td>
            <td>
                <div class="item-name-cell">${item.name}</div>
            </td>
            <td>
                <span class="oop-badge ${typeBadgeClass}"><i data-lucide="${typeIcon}"></i> ${item.type}</span>
            </td>
            <td>
                <span class="badge-tag">${item.category}</span>
            </td>
            <td>
                <div class="stock-health-wrap">
                    <div class="stock-count-row">
                        <span class="stock-count-num">${item.quantity} units</span>
                        <span style="font-size:0.72rem; color:${item.quantity <= 5 ? 'var(--amber-600)' : 'var(--text-muted)'}; font-weight:600;">${stockLabel}</span>
                    </div>
                    <div class="progress-bar-track">
                        <div class="progress-fill ${fillClass}" style="width: ${pct}%;"></div>
                    </div>
                </div>
            </td>
            <td class="price-inr">${formatINR(item.price)}</td>
            <td class="val-inr">${formatINR(item.valuation)}</td>
            <td>${specialDetails}</td>
            <td class="text-right">
                <div class="row-actions">
                    <button class="btn-icon-action action-in" title="Quick Stock-In (Replenish)" onclick="openStockInModal(${item.id})">
                        <i data-lucide="plus"></i>
                    </button>
                    <button class="btn-icon-action action-out" title="Quick Stock-Out (Dispatch)" onclick="openStockOutModal(${item.id})">
                        <i data-lucide="minus"></i>
                    </button>
                    <button class="btn-icon-action action-del" title="Delete Product" onclick="deleteItem(${item.id})">
                        <i data-lucide="trash-2"></i>
                    </button>
                </div>
            </td>
        `;

        catalogTableBody.appendChild(tr);
    });

    document.getElementById('tableSummaryText').innerText = `Showing ${filtered.length} of ${inventory.length} products`;
    lucide.createIcons();
}

// Render Low Stock View
function renderLowStockView() {
    const lowItems = inventory.filter(i => i.quantity <= 5);
    lowStockCardsGrid.innerHTML = '';

    if (lowItems.length === 0) {
        lowStockCardsGrid.innerHTML = `
            <div style="grid-column: 1/-1; text-align: center; padding: 50px 20px; color: var(--emerald-600);">
                <i data-lucide="shield-check" style="width: 52px; height: 52px; display: block; margin: 0 auto 14px;"></i>
                <h3 style="font-size: 1.2rem; font-weight: 700;">Inventory Well Stocked</h3>
                <p style="color: var(--text-muted); font-size: 0.9rem; margin-top: 4px;">All SKUs are comfortably above the safety reorder threshold of 5 units.</p>
            </div>
        `;
        lucide.createIcons();
        return;
    }

    lowItems.forEach(item => {
        const card = document.createElement('div');
        card.className = 'reorder-product-card';
        card.innerHTML = `
            <div>
                <div style="display: flex; justify-content: space-between; align-items: flex-start; margin-bottom: 8px;">
                    <div>
                        <span class="item-id-cell">#${item.id}</span>
                        <h4 style="font-size: 1.1rem; font-weight: 700; margin-top: 2px;">${item.name}</h4>
                    </div>
                    <span class="reorder-badge-pill">Only ${item.quantity} units left</span>
                </div>
                <p style="font-size: 0.84rem; color: var(--text-muted);">
                    Category: <strong>${item.category}</strong> &bull; Type: <strong>${item.type}</strong>
                </p>
                <p style="font-size: 0.95rem; font-weight: 700; margin-top: 8px; color: var(--text-main);">
                    Unit Price: <span class="price-inr">${formatINR(item.price)}</span>
                </p>
            </div>
            <button class="btn btn-emerald" style="width: 100%; justify-content: center;" onclick="openStockInModal(${item.id})">
                <i data-lucide="arrow-down-left"></i> Restock Now
            </button>
        `;
        lowStockCardsGrid.appendChild(card);
    });
    lucide.createIcons();
}

// Render Transaction Audit Trail
function renderTransactions() {
    const filter = (document.getElementById('txFilterInput')?.value || '').trim().toLowerCase();
    txTableBody.innerHTML = '';

    const list = transactions.filter(t => {
        if (!filter) return true;
        return t.itemName.toLowerCase().includes(filter) ||
               t.itemId.toString().includes(filter) ||
               t.type.toLowerCase().includes(filter) ||
               t.notes.toLowerCase().includes(filter);
    });

    if (list.length === 0) {
        txTableBody.innerHTML = `<tr><td colspan="8" style="text-align: center; padding: 36px; color: var(--text-muted);">No transaction records match filter.</td></tr>`;
        return;
    }

    list.forEach(tx => {
        const tr = document.createElement('tr');

        let typeBadge = `<span class="oop-badge badge-standard">${tx.type}</span>`;
        let qtyColor = 'var(--text-main)';
        let qtyText = tx.quantityChange > 0 ? `+${tx.quantityChange}` : tx.quantityChange;

        if (tx.type === 'STOCK_IN') {
            typeBadge = `<span class="oop-badge badge-perishable"><i data-lucide="arrow-down-left"></i> STOCK_IN</span>`;
            qtyColor = 'var(--emerald-600)';
        } else if (tx.type === 'STOCK_OUT') {
            typeBadge = `<span class="oop-badge" style="background:var(--red-50); color:var(--red-600);"><i data-lucide="arrow-up-right"></i> STOCK_OUT</span>`;
            qtyColor = 'var(--red-600)';
        } else if (tx.type === 'INITIAL') {
            typeBadge = `<span class="oop-badge badge-standard"><i data-lucide="sparkles"></i> INITIAL</span>`;
        }

        tr.innerHTML = `
            <td class="item-id-cell">#${tx.txId}</td>
            <td style="font-size:0.8rem; color:var(--text-muted);">${tx.timestamp}</td>
            <td class="item-id-cell">#${tx.itemId}</td>
            <td style="font-weight:600;">${tx.itemName}</td>
            <td>${typeBadge}</td>
            <td style="font-family:'JetBrains Mono',monospace; font-weight:700; color:${qtyColor};">${qtyText}</td>
            <td style="font-family:'JetBrains Mono',monospace; font-weight:700;">${tx.resultingStock}</td>
            <td style="font-size:0.85rem; color:var(--text-muted);">${tx.notes || '&mdash;'}</td>
        `;
        txTableBody.appendChild(tr);
    });
    lucide.createIcons();
}

// Render Interactive Analytics Charts (Chart.js)
function renderCharts() {
    const isDark = document.documentElement.getAttribute('data-theme') === 'dark';
    const textColor = isDark ? '#cbd5e1' : '#475569';
    const gridColor = isDark ? '#1e293b' : '#f1f5f9';

    // Calculate aggregated category valuations
    const catValMap = {};
    const catCountMap = {};

    inventory.forEach(item => {
        const cat = item.category || 'General';
        catValMap[cat] = (catValMap[cat] || 0) + item.valuation;
        catCountMap[cat] = (catCountMap[cat] || 0) + 1;
    });

    const labels = Object.keys(catValMap);
    const valData = Object.values(catValMap);
    const countData = labels.map(l => catCountMap[l]);

    // Chart 1: Valuation Bar Chart
    const ctxVal = document.getElementById('categoryValueChart')?.getContext('2d');
    if (ctxVal) {
        if (categoryValueChartInstance) categoryValueChartInstance.destroy();
        categoryValueChartInstance = new Chart(ctxVal, {
            type: 'bar',
            data: {
                labels: labels,
                datasets: [{
                    label: 'Valuation (₹ INR)',
                    data: valData,
                    backgroundColor: [
                        '#4f46e5', '#10b981', '#8b5cf6', '#f59e0b', '#06b6d4', '#ec4899'
                    ],
                    borderRadius: 8
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                plugins: {
                    legend: { display: false },
                    tooltip: {
                        callbacks: {
                            label: (ctx) => `Valuation: ${formatINR(ctx.raw)}`
                        }
                    }
                },
                scales: {
                    x: {
                        ticks: { color: textColor, font: { family: 'Plus Jakarta Sans', weight: '600' } },
                        grid: { display: false }
                    },
                    y: {
                        ticks: {
                            color: textColor,
                            font: { family: 'JetBrains Mono' },
                            callback: (v) => `₹${v.toLocaleString('en-IN')}`
                        },
                        grid: { color: gridColor }
                    }
                }
            }
        });
    }

    // Chart 2: Product Count Doughnut Chart
    const ctxCount = document.getElementById('categoryCountChart')?.getContext('2d');
    if (ctxCount) {
        if (categoryCountChartInstance) categoryCountChartInstance.destroy();
        categoryCountChartInstance = new Chart(ctxCount, {
            type: 'doughnut',
            data: {
                labels: labels,
                datasets: [{
                    data: countData,
                    backgroundColor: [
                        '#4f46e5', '#10b981', '#8b5cf6', '#f59e0b', '#06b6d4', '#ec4899'
                    ],
                    borderWidth: 0
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                plugins: {
                    legend: {
                        position: 'bottom',
                        labels: { color: textColor, font: { family: 'Plus Jakarta Sans', size: 12, weight: '600' }, padding: 16 }
                    }
                },
                cutout: '68%'
            }
        });
    }
}

// Navigation Tabs
function initNavigation() {
    const navItems = document.querySelectorAll('.nav-item');
    const views = {
        'dashboard': document.getElementById('viewCatalog'),
        'catalog': document.getElementById('viewCatalog'),
        'low-stock': document.getElementById('viewLowStock'),
        'transactions': document.getElementById('viewTransactions'),
        'analytics': document.getElementById('viewAnalytics'),
        'architecture': document.getElementById('viewArchitecture')
    };

    const titles = {
        'dashboard': { title: 'Inventory Dashboard', sub: 'Real-time stock valuation & asset management in Indian Rupees (₹)' },
        'catalog': { title: 'Product Catalog', sub: 'Comprehensive inventory list with instant stock operations' },
        'low-stock': { title: 'Low Stock Reorder Hub', sub: 'Replenishment warnings for items approaching zero stock' },
        'transactions': { title: 'Audit Trail Ledger', sub: 'Immutable record of inward/outward movements & updates' },
        'analytics': { title: 'Portfolio Analytics', sub: 'Visual breakdown of asset distribution and SKU density' },
        'architecture': { title: 'OOP & UML Design Specs', sub: 'Inheritance, Polymorphism, STL Containers & Exception Architecture' }
    };

    navItems.forEach(item => {
        item.addEventListener('click', () => {
            navItems.forEach(n => n.classList.remove('active'));
            item.classList.add('active');

            const tab = item.dataset.tab;
            Object.values(views).forEach(v => v.classList.remove('active'));
            if (views[tab]) views[tab].classList.add('active');

            if (titles[tab]) {
                document.getElementById('navbarTitle').innerText = titles[tab].title;
                document.getElementById('navbarSubtitle').innerText = titles[tab].sub;
            }

            if (tab === 'catalog' || tab === 'dashboard') renderCatalog();
            if (tab === 'low-stock') renderLowStockView();
            if (tab === 'transactions') renderTransactions();
            if (tab === 'analytics') renderCharts();
        });
    });
}

// Category Chips
function initCategoryChips() {
    const chips = document.querySelectorAll('.chip-filter');
    chips.forEach(chip => {
        chip.addEventListener('click', () => {
            chips.forEach(c => c.classList.remove('active'));
            chip.classList.add('active');
            activeCategoryFilter = chip.dataset.filter;
            renderCatalog();
        });
    });
}

// Search & Sort Event Listeners
function initSearchAndSort() {
    catalogSearchInput.addEventListener('input', () => {
        const val = catalogSearchInput.value.length > 0;
        document.getElementById('btnClearSearch').style.display = val ? 'block' : 'none';
        renderCatalog();
    });

    document.getElementById('btnClearSearch').addEventListener('click', () => {
        catalogSearchInput.value = '';
        document.getElementById('btnClearSearch').style.display = 'none';
        renderCatalog();
    });

    catalogTypeSelect.addEventListener('change', renderCatalog);
    catalogSortSelect.addEventListener('change', renderCatalog);

    document.getElementById('txFilterInput')?.addEventListener('input', renderTransactions);

    // Quick search from sidebar
    document.getElementById('sidebarQuickSearch')?.addEventListener('input', (e) => {
        catalogSearchInput.value = e.target.value;
        renderCatalog();
        // Switch to catalog tab
        document.querySelector('.nav-item[data-tab="catalog"]')?.click();
    });
}

// Keyboard Shortcuts
function initShortcuts() {
    window.addEventListener('keydown', (e) => {
        // Press '/' to search
        if (e.key === '/' && document.activeElement.tagName !== 'INPUT' && document.activeElement.tagName !== 'SELECT') {
            e.preventDefault();
            catalogSearchInput.focus();
        }
        // Press Alt+N for New Item
        if (e.altKey && (e.key === 'n' || e.key === 'N')) {
            e.preventDefault();
            document.getElementById('btnOpenAddModal').click();
        }
        // Escape closes modals
        if (e.key === 'Escape') {
            modalAdd.classList.remove('open');
            modalStockIn.classList.remove('open');
            modalStockOut.classList.remove('open');
        }
    });
}

// Modals Handling
function initModals() {
    // Open Add Modal
    document.getElementById('btnOpenAddModal').addEventListener('click', () => {
        modalAdd.classList.add('open');
        const nextId = inventory.length > 0 ? Math.max(...inventory.map(i => i.id)) + 1 : 105;
        document.getElementById('addId').value = nextId;
        updateValuationPreview();
    });

    document.getElementById('btnCloseAddModal').addEventListener('click', () => modalAdd.classList.remove('open'));
    document.getElementById('btnCancelAdd').addEventListener('click', () => modalAdd.classList.remove('open'));

    // Dynamic field toggle
    document.getElementById('addType').addEventListener('change', (e) => {
        const type = e.target.value;
        document.getElementById('addPerishableFields').style.display = (type === 'Perishable') ? 'block' : 'none';
        document.getElementById('addElectronicsFields').style.display = (type === 'Electronics') ? 'block' : 'none';
    });

    // Dynamic Valuation Preview
    const addQtyInput = document.getElementById('addQuantity');
    const addPriceInput = document.getElementById('addPrice');

    function updateValuationPreview() {
        const q = parseInt(addQtyInput.value, 10) || 0;
        const p = parseFloat(addPriceInput.value) || 0;
        document.getElementById('addValuationPreview').innerText = formatINR(q * p);
    }

    addQtyInput.addEventListener('input', updateValuationPreview);
    addPriceInput.addEventListener('input', updateValuationPreview);

    // Add Form Submit
    document.getElementById('formAddItem').addEventListener('submit', async (e) => {
        e.preventDefault();
        const type = document.getElementById('addType').value;
        const payload = {
            id: parseInt(document.getElementById('addId').value, 10),
            name: document.getElementById('addName').value.trim(),
            category: document.getElementById('addCategory').value.trim(),
            quantity: parseInt(document.getElementById('addQuantity').value, 10),
            price: parseFloat(document.getElementById('addPrice').value),
            type: type
        };

        if (type === 'Perishable') {
            payload.expiryDate = document.getElementById('addExpiry').value || '2026-12-31';
            payload.storageTemp = parseFloat(document.getElementById('addTemp').value) || 4.0;
        } else if (type === 'Electronics') {
            payload.warrantyMonths = parseInt(document.getElementById('addWarranty').value, 10) || 12;
            payload.brand = document.getElementById('addBrand').value.trim() || 'Generic';
        }

        try {
            const res = await fetch('/api/items', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(payload)
            });
            const data = await res.json();
            if (!res.ok) throw new Error(data.error || 'Failed to create product');

            showToast(`Product '${payload.name}' added successfully (${formatINR(payload.price)})!`, 'success');
            modalAdd.classList.remove('open');
            document.getElementById('formAddItem').reset();
            fetchData();
        } catch (err) {
            showToast(err.message, 'danger');
        }
    });

    // Stock-In Modal
    document.getElementById('btnCloseStockIn').addEventListener('click', () => modalStockIn.classList.remove('open'));
    document.getElementById('btnCancelStockIn').addEventListener('click', () => modalStockIn.classList.remove('open'));

    const stockInQtyInput = document.getElementById('stockInQty');
    stockInQtyInput.addEventListener('input', () => {
        const cur = parseInt(document.getElementById('stockInCurrentCount').innerText, 10) || 0;
        const add = parseInt(stockInQtyInput.value, 10) || 0;
        document.getElementById('stockInIncomingPreview').innerText = add;
        document.getElementById('stockInTotalPreview').innerText = cur + add;
    });

    document.getElementById('formStockIn').addEventListener('submit', async (e) => {
        e.preventDefault();
        const id = document.getElementById('stockInItemId').value;
        const amount = parseInt(document.getElementById('stockInQty').value, 10);
        const notes = document.getElementById('stockInNotes').value.trim();

        try {
            const res = await fetch(`/api/items/${id}/stock-in`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ amount, notes })
            });
            const data = await res.json();
            if (!res.ok) throw new Error(data.error || 'Stock-in failed');

            showToast(`Replenished +${amount} units for #${id}.`, 'success');
            modalStockIn.classList.remove('open');
            document.getElementById('formStockIn').reset();
            fetchData();
        } catch (err) {
            showToast(err.message, 'danger');
        }
    });

    // Stock-Out Modal (Negative Stock Protection UX)
    document.getElementById('btnCloseStockOut').addEventListener('click', () => modalStockOut.classList.remove('open'));
    document.getElementById('btnCancelStockOut').addEventListener('click', () => modalStockOut.classList.remove('open'));

    const stockOutQtyInput = document.getElementById('stockOutQty');
    const stockOutErrorMsg = document.getElementById('stockOutErrorMsg');
    const stockOutErrorText = document.getElementById('stockOutErrorText');
    const btnSubmitStockOut = document.getElementById('btnSubmitStockOut');

    stockOutQtyInput.addEventListener('input', () => {
        const available = parseInt(document.getElementById('stockOutAvailableCount').innerText, 10) || 0;
        const dispatch = parseInt(stockOutQtyInput.value, 10) || 0;
        const resulting = available - dispatch;

        document.getElementById('stockOutDispatchPreview').innerText = dispatch;
        document.getElementById('stockOutResultPreview').innerText = resulting >= 0 ? resulting : 0;

        // CRITICAL CHECK: Negative Stock Protection UX
        if (dispatch > available) {
            stockOutErrorText.innerText = `Insufficient stock! Requested (${dispatch}) > Available (${available}). Negative stock prevented!`;
            stockOutErrorMsg.style.display = 'flex';
            btnSubmitStockOut.disabled = true;
            btnSubmitStockOut.style.opacity = '0.5';
            btnSubmitStockOut.style.cursor = 'not-allowed';
            document.getElementById('stockOutResultPreview').style.color = 'var(--red-600)';
        } else {
            stockOutErrorMsg.style.display = 'none';
            btnSubmitStockOut.disabled = false;
            btnSubmitStockOut.style.opacity = '1';
            btnSubmitStockOut.style.cursor = 'pointer';
            document.getElementById('stockOutResultPreview').style.color = 'var(--indigo-600)';
        }
    });

    document.getElementById('formStockOut').addEventListener('submit', async (e) => {
        e.preventDefault();
        const id = document.getElementById('stockOutItemId').value;
        const amount = parseInt(document.getElementById('stockOutQty').value, 10);
        const notes = document.getElementById('stockOutNotes').value.trim();

        try {
            const res = await fetch(`/api/items/${id}/stock-out`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ amount, notes })
            });
            const data = await res.json();
            if (!res.ok) throw new Error(data.error || 'Stock-out rejected');

            showToast(`Dispatched -${amount} units for #${id}.`, 'success');
            modalStockOut.classList.remove('open');
            document.getElementById('formStockOut').reset();
            fetchData();
        } catch (err) {
            showToast(err.message, 'danger');
        }
    });

    // CSV Export
    document.getElementById('btnExportCSV').addEventListener('click', exportCSV);
}

// Open Quick Stock-In
window.openStockInModal = function(id) {
    const item = inventory.find(i => i.id === id);
    if (!item) return;

    document.getElementById('stockInItemId').value = item.id;
    document.getElementById('stockInItemHeader').innerText = `Replenish: ${item.name} (#${item.id})`;
    document.getElementById('stockInCurrentCount').innerText = item.quantity;
    document.getElementById('stockInIncomingPreview').innerText = '0';
    document.getElementById('stockInTotalPreview').innerText = item.quantity;
    document.getElementById('stockInQty').value = '';
    document.getElementById('stockInNotes').value = '';

    modalStockIn.classList.add('open');
    document.getElementById('stockInQty').focus();
};

// Open Quick Stock-Out
window.openStockOutModal = function(id) {
    const item = inventory.find(i => i.id === id);
    if (!item) return;

    document.getElementById('stockOutItemId').value = item.id;
    document.getElementById('stockOutItemHeader').innerText = `Dispatch: ${item.name} (#${item.id})`;
    document.getElementById('stockOutAvailableCount').innerText = item.quantity;
    document.getElementById('stockOutDispatchPreview').innerText = '0';
    document.getElementById('stockOutResultPreview').innerText = item.quantity;
    document.getElementById('stockOutQty').value = '';
    document.getElementById('stockOutNotes').value = '';
    document.getElementById('stockOutErrorMsg').style.display = 'none';

    btnSubmitStockOut.disabled = false;
    btnSubmitStockOut.style.opacity = '1';
    btnSubmitStockOut.style.cursor = 'pointer';

    modalStockOut.classList.add('open');
    document.getElementById('stockOutQty').focus();
};

// Delete Product
window.deleteItem = async function(id) {
    const item = inventory.find(i => i.id === id);
    if (!confirm(`Are you sure you want to delete '${item?.name}' (ID #${id}) from the catalog?`)) {
        return;
    }

    try {
        const res = await fetch(`/api/items/${id}`, { method: 'DELETE' });
        const data = await res.json();
        if (!res.ok) throw new Error(data.error || 'Failed to delete');

        showToast(`Item #${id} deleted from inventory.`, 'info');
        fetchData();
    } catch (err) {
        showToast(err.message, 'danger');
    }
};

// Export CSV
function exportCSV() {
    if (inventory.length === 0) {
        showToast('Catalog is empty, nothing to export.', 'danger');
        return;
    }

    let csv = 'Type,ID,Name,Quantity,Price,Category,Extra1,Extra2\n';
    inventory.forEach(i => {
        let e1 = 'N/A', e2 = 'N/A';
        if (i.type === 'Perishable') {
            e1 = i.expiryDate || '2026-12-31';
            e2 = i.storageTemp !== undefined ? i.storageTemp : '4.0';
        } else if (i.type === 'Electronics') {
            e1 = i.warrantyMonths || '12';
            e2 = i.brand || 'Generic';
        }
        csv += `${i.type},${i.id},"${i.name}",${i.quantity},${i.price.toFixed(2)},"${i.category}",${e1},${e2}\n`;
    });

    const blob = new Blob([csv], { type: 'text/csv;charset=utf-8;' });
    const url = URL.createObjectURL(blob);
    const link = document.createElement('a');
    link.href = url;
    link.download = `StockMatrix_INR_Inventory_${new Date().toISOString().slice(0, 10)}.csv`;
    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);
    showToast('Inventory CSV exported in INR (₹)!', 'success');
}
